#include <iostream>
#include <functional>
#include <vector>

#include "equi_miner.h"
#include "cpu_tromp.hpp"


void CPU_TROMP::start(CPU_TROMP& device_context) {
	device_context._eq = new equi();
}

void CPU_TROMP::stop(CPU_TROMP& device_context) {
	delete device_context._eq;
}

bool CPU_TROMP::solve(const char *tequihash_header,
	unsigned int tequihash_header_len,
	const char* nonce,
	unsigned int nonce_len,
	std::function<bool()> cancelf,
	std::function<bool(const std::vector<uint32_t>&, size_t, const unsigned char*)> solutionf,
	std::function<void(void)> hashdonef,
	CPU_TROMP& device_context)
{
	device_context._eq->setnonce(tequihash_header, tequihash_header_len, nonce, nonce_len);
	device_context._eq->digit0(0);
	device_context._eq->xfull = device_context._eq->bfull = device_context._eq->hfull = 0;
	u32 r = 1;

	for (; r < WK; r++) {
		if (cancelf()) return false;
		r & 1 ? device_context._eq->digitodd(r, 0) : device_context._eq->digiteven(r, 0);
		device_context._eq->xfull = device_context._eq->bfull = device_context._eq->hfull = 0;
	}

	if (cancelf()) return false;

	device_context._eq->digitK(0);

	for (unsigned s = 0; s < device_context._eq->nsols; s++)
	{
		std::vector<uint32_t> index_vector(PROOFSIZE);
		for (u32 i = 0; i < PROOFSIZE; i++) {
			index_vector[i] = device_context._eq->sols[s][i];
		}

		if (solutionf(index_vector, DIGITBITS, nullptr))
		{
			hashdonef();
			return true;
		}
		if (cancelf()) return false;
	}
	hashdonef();
	return false;
}

void equi_setheader(crypto_generichash_blake2b_state *ctx, const char *header, const u32 headerLen, const char* nce, const u32 nonceLen)
{
  EhInitialiseState(WN, WK, *ctx);

  crypto_generichash_blake2b_update(ctx, (const uchar *)header, headerLen);
  crypto_generichash_blake2b_update(ctx, (const uchar *)nce, nonceLen);
}
