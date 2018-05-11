#include "ocl_silentarmy.hpp"

//#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <sys/types.h>
//#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <unistd.h>
//#include <getopt.h>
#include <errno.h>

#include "CL/opencl.h"

#include <boost/compute/core.hpp>

#include <fstream>
#include "sa_blake.h"

typedef uint8_t		uchar;
typedef uint32_t	uint;
typedef uint64_t	ulong;
#include "param.h"

#define MIN(A, B)	(((A) < (B)) ? (A) : (B))
#define MAX(A, B)	(((A) > (B)) ? (A) : (B))

#define WN PARAM_N
#define WK PARAM_K

#define COLLISION_BIT_LENGTH (WN / (WK+1))
#define COLLISION_BYTE_LENGTH ((COLLISION_BIT_LENGTH+7)/8)
#define FINAL_FULL_WIDTH (2*COLLISION_BYTE_LENGTH+sizeof(uint32_t)*(1 << (WK)))

#define NDIGITS   (WK+1)
#define DIGITBITS (WN/(NDIGITS))
#define PROOFSIZE (1u<<WK)
#define COMPRESSED_PROOFSIZE ((COLLISION_BIT_LENGTH+1)*PROOFSIZE*4/(8*sizeof(uint32_t)))

#include "_kernel.h"

typedef struct  debug_s
{
	uint32_t    dropped_coll;
	uint32_t    dropped_stor;
}               debug_t;

struct OclContext {
	boost::compute::device cldevice;

	boost::compute::platform platform;

	boost::compute::context clcontext;
	boost::compute::program clprogram;

	boost::compute::command_queue queue;
	boost::compute::kernel k_init_ht;
	boost::compute::kernel k_rounds[PARAM_K];
	boost::compute::kernel k_sols;

	boost::compute::buffer buf_ht[2], buf_sols, buf_dbg;

	size_t global_ws;
	size_t local_work_size = 64;

	sols_t	*sols;

	OclContext(boost::compute::device&& device, unsigned threadsNum, unsigned threadsPerBlock);

	~OclContext() {
		free(sols);
	}
};

cl_mem check_clCreateBuffer(cl_context ctx, cl_mem_flags flags, size_t size,
	void *host_ptr);

OclContext::OclContext(boost::compute::device && device, unsigned int threadsNum, unsigned int threadsPerBlock)
	: cldevice(device)
	, platform(cldevice.platform())
{
	this->clcontext = boost::compute::context(cldevice);
	this->queue = boost::compute::command_queue(clcontext, cldevice);

#ifdef ENABLE_DEBUG
    size_t              dbg_size = NR_ROWS * sizeof (debug_t);
#else
    size_t              dbg_size = 1 * sizeof (debug_t);
#endif

	buf_dbg = boost::compute::buffer(clcontext, dbg_size);
	buf_ht[0] = boost::compute::buffer(clcontext, HT_SIZE);
	buf_ht[1] = boost::compute::buffer(clcontext, HT_SIZE);
	buf_sols = boost::compute::buffer(clcontext, sizeof(sols_t));

	fprintf(stderr, "Hash tables will use %.1f MB\n", 2.0 * HT_SIZE / 1e6);

	clprogram = boost::compute::program::create_with_source(std::string(ocl_code), clcontext);

	clprogram.build();

	k_init_ht = clprogram.create_kernel("kernel_init_ht");
	for (unsigned i = 0; i < WK; i++) {
		char kernelName[128];
		sprintf(kernelName, "kernel_round%d", i);
		k_rounds[i] = clprogram.create_kernel(kernelName);
	}

	sols = (sols_t *)malloc(sizeof(*sols));

	k_sols = clprogram.create_kernel("kernel_sols");
}

///
int             verbose = 0;
uint32_t	show_encoded = 0;

void hexdump(uint8_t *a, uint32_t a_len)
{
	for (uint32_t i = 0; i < a_len; i++)
		fprintf(stderr, "%02x", a[i]);
}

char *s_hexdump(const void *_a, uint32_t a_len)
{
	const uint8_t	*a = (uint8_t	*)_a;
	static char		buf[1024];
	uint32_t		i;
	for (i = 0; i < a_len && i + 2 < sizeof(buf); i++)
		sprintf(buf + i * 2, "%02x", a[i]);
	buf[i * 2] = 0;
	return buf;
}

uint8_t hex2val(const char *base, size_t off)
{
	const char          c = base[off];
	if (c >= '0' && c <= '9')           return c - '0';
	else if (c >= 'a' && c <= 'f')      return 10 + c - 'a';
	else if (c >= 'A' && c <= 'F')      return 10 + c - 'A';
	printf("Invalid hex char at offset %zd: ...%c...\n", off, c);
	return 0;
}

static void compress(uint8_t *out, uint32_t *inputs, uint32_t n)
{
	uint32_t byte_pos = 0;
	int32_t bits_left = PREFIX + 1;
	uint8_t x = 0;
	uint8_t x_bits_used = 0;
	uint8_t *pOut = out;
	while (byte_pos < n)
	{
		if (bits_left >= 8 - x_bits_used)
		{
			x |= inputs[byte_pos] >> (bits_left - 8 + x_bits_used);
			bits_left -= 8 - x_bits_used;
			x_bits_used = 8;
		}
		else if (bits_left > 0)
		{
			uint32_t mask = ~(-1 << (8 - x_bits_used));
			mask = ((~mask) >> bits_left) & mask;
			x |= (inputs[byte_pos] << (8 - x_bits_used - bits_left)) & mask;
			x_bits_used += bits_left;
			bits_left = 0;
		}
		else if (bits_left <= 0)
		{
			assert(!bits_left);
			byte_pos++;
			bits_left = PREFIX + 1;
		}
		if (x_bits_used == 8)
		{
			*pOut++ = x;
			x = x_bits_used = 0;
		}
	}
}

void get_program_build_log(cl_program program, cl_device_id device)
{
	cl_int		status;
	char	        val[2 * 1024 * 1024];
	size_t		ret = 0;
	status = clGetProgramBuildInfo(program, device,
		CL_PROGRAM_BUILD_LOG,
		sizeof(val),	// size_t param_value_size
		&val,		// void *param_value
		&ret);		// size_t *param_value_size_ret
	if (status != CL_SUCCESS)
		printf("clGetProgramBuildInfo (%d)\n", status);
	fprintf(stderr, "%s\n", val);
}

size_t select_work_size_blake(int nr_compute_units)
{
	size_t              work_size =
		64 * /* thread per wavefront */
		BLAKE_WPS * /* wavefront per simd */
		4 * /* simd per compute unit */
		nr_compute_units;
	// Make the work group size a multiple of the nr of wavefronts, while
	// dividing the number of inputs. This results in the worksize being a
	// power of 2.
	while (NR_INPUTS % work_size)
		work_size += 64;
	//debug("Blake: work size %zd\n", work_size);
	return work_size;
}

static void init_ht(boost::compute::command_queue queue, boost::compute::kernel k_init_ht, boost::compute::buffer buf_ht)
{
	size_t      global_ws = NR_ROWS;
	size_t      local_ws = 64;
	cl_int      status;
#if 0
	uint32_t    pat = -1;
	status = clEnqueueFillBuffer(queue, buf_ht, &pat, sizeof(pat), 0,
		NR_ROWS * NR_SLOTS * SLOT_LEN,
		0,		// cl_uint	num_events_in_wait_list
		NULL,	// cl_event	*event_wait_list
		NULL);	// cl_event	*event
	if (status != CL_SUCCESS)
		fatal("clEnqueueFillBuffer (%d)\n", status);
#endif
	k_init_ht.set_arg(0, buf_ht);
	if (status != CL_SUCCESS)
		printf("clSetKernelArg (%d)\n", status);
	queue.enqueue_nd_range_kernel(k_init_ht,
		1,		// cl_uint	work_dim
		NULL,	// size_t	*global_work_offset
		&global_ws,	// size_t	*global_work_size
		&local_ws);	// size_t	*local_work_size
}


/*
** Sort a pair of binary blobs (a, b) which are consecutive in memory and
** occupy a total of 2*len 32-bit words.
**
** a            points to the pair
** len          number of 32-bit words in each pair
*/
void sort_pair(uint32_t *a, uint32_t len)
{
	uint32_t    *b = a + len;
	uint32_t     tmp, need_sorting = 0;
	for (uint32_t i = 0; i < len; i++)
		if (need_sorting || a[i] > b[i])
		{
			need_sorting = 1;
			tmp = a[i];
			a[i] = b[i];
			b[i] = tmp;
		}
		else if (a[i] < b[i])
			return;
}
static uint32_t verify_sol(sols_t *sols, unsigned sol_i)
{
	uint32_t  *inputs = sols->values[sol_i];
	uint32_t  seen_len = (1 << (PREFIX + 1)) / 8;
	uint8_t seen[(1 << (PREFIX + 1)) / 8];
	uint32_t  i;
	uint8_t tmp;
	// look for duplicate inputs
	memset(seen, 0, seen_len);
	for (i = 0; i < (1 << PARAM_K); i++)
	{
		tmp = seen[inputs[i] / 8];
		seen[inputs[i] / 8] |= 1 << (inputs[i] & 7);
		if (tmp == seen[inputs[i] / 8])
		{
			// at least one input value is a duplicate
			sols->valid[sol_i] = 0;
			return 0;
		}
	}
	// the valid flag is already set by the GPU, but set it again because
	// I plan to change the GPU code to not set it
	sols->valid[sol_i] = 1;
	// sort the pairs in place
	for (uint32_t level = 0; level < PARAM_K; level++)
		for (i = 0; i < (1 << PARAM_K); i += (2 << level))
			sort_pair(&inputs[i], 1 << level);
	return 1;
}



ocl_silentarmy::ocl_silentarmy(int platf_id, int dev_id) {
	platform_id = platf_id;
	device_id = dev_id;
	// TODO
	threadsNum = 8192;
	wokrsize = 128; // 256;
}

std::string ocl_silentarmy::getdevinfo() {
	/*TODO get name*/
	return "GPU_ID(" + std::to_string(device_id)+ ")";
}

// STATICS START
int ocl_silentarmy::getcount() { /*TODO*/
	return 0;
}

void ocl_silentarmy::getinfo(int platf_id, int d_id, std::string& gpu_name, int& sm_count, std::string& version) { /*TODO*/ }

void ocl_silentarmy::start(ocl_silentarmy& device_context) {
	/*TODO*/
	device_context.is_init_success = false;

	auto devices = boost::compute::system::devices();

	boost::compute::device& device = devices[device_context.device_id];

	printf("Using device %s\n", device.name().c_str());

	device_context.oclc = new OclContext(std::move(device), device_context.threadsNum, device_context.wokrsize);

	device_context.is_init_success = true;
}

void ocl_silentarmy::stop(ocl_silentarmy& device_context) {
	if (device_context.oclc != nullptr) delete device_context.oclc;
}

bool ocl_silentarmy::solve(const char *tequihash_header,
	unsigned int tequihash_header_len,
	const char* nonce,
	unsigned int nonce_len,
	std::function<bool()> cancelf,
	std::function<bool(const std::vector<uint32_t>&, size_t, const unsigned char*)> solutionf,
	std::function<void(void)> hashdonef,
	ocl_silentarmy& device_context) {

	unsigned char context[ZCASH_BLOCK_HEADER_LEN];
	memset(context, 0, ZCASH_BLOCK_HEADER_LEN);
	memcpy(context, tequihash_header, tequihash_header_len);
	memcpy(context + tequihash_header_len, nonce, nonce_len);

	OclContext *miner = device_context.oclc;
	clFlush(miner->queue);

	blake2b_state_t initialCtx;
	zcash_blake2b_init(&initialCtx, ZCASH_HASH_LEN, PARAM_N, PARAM_K);
	zcash_blake2b_update(&initialCtx, (const uint8_t*)context, 128, 0);

	boost::compute::buffer buf_blake_st = boost::compute::buffer(miner->clcontext,
		sizeof(blake2b_state_s), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &initialCtx);

	for (unsigned round = 0; round < PARAM_K; round++)
	{
		if (round < 2)
			init_ht(miner->queue, miner->k_init_ht, miner->buf_ht[round % 2]);
		if (!round)
		{
			miner->k_rounds[round].set_arg(0, buf_blake_st);
			miner->k_rounds[round].set_arg(1, miner->buf_ht[round % 2]);
			miner->global_ws = select_work_size_blake(miner->cldevice.compute_units());
		}
		else
		{
			miner->k_rounds[round].set_arg(0, miner->buf_ht[(round - 1) % 2]);
			miner->k_rounds[round].set_arg(1, miner->buf_ht[round % 2]);
			miner->global_ws = NR_ROWS;
		}
		miner->k_rounds[round].set_arg(2, miner->buf_dbg);
		if (round == PARAM_K - 1)
			miner->k_rounds[round].set_arg(3, miner->buf_sols);

		miner->queue.enqueue_nd_range_kernel(miner->k_rounds[round], 1, NULL,
			&miner->global_ws, &miner->local_work_size);
		// cancel function
		if (cancelf())
			return false;
	}
	miner->k_sols.set_arg(0, miner->buf_ht[0]);
	miner->k_sols.set_arg(1, miner->buf_ht[1]);
	miner->k_sols.set_arg(2, miner->buf_sols);
	miner->global_ws = NR_ROWS;
	miner->queue.enqueue_nd_range_kernel(miner->k_sols, 1, NULL,
		&miner->global_ws, &miner->local_work_size);

	miner->queue.enqueue_read_buffer(miner->buf_sols,
		0,		// size_t	offset
		sizeof(*miner->sols),	// size_t	size
		miner->sols);// void		*ptr

	if (miner->sols->nr > MAX_SOLS)
		miner->sols->nr = MAX_SOLS;

	for (unsigned sol_i = 0; sol_i < miner->sols->nr; sol_i++) {
		verify_sol(miner->sols, sol_i);
	}

	uint8_t proof[COMPRESSED_PROOFSIZE * 2];
	for (uint32_t i = 0; i < miner->sols->nr; i++) {
		if (miner->sols->valid[i]) {
			compress(proof, (uint32_t *)(miner->sols->values[i]), 1 << PARAM_K);
			if (solutionf(std::vector<uint32_t>(0), 1344, proof))
				return true;
		}
	}
	hashdonef();
	return false;
}

// STATICS END

