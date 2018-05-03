#include <iostream>
#include <functional>
#include <vector>

#include "equi_miner.h"
#include "cpu_tromp.hpp"


void CPU_TROMP::start(CPU_TROMP& device_context) { }

void CPU_TROMP::stop(CPU_TROMP& device_context) { }

bool CPU_TROMP::solve(const char *tequihash_header,
	unsigned int tequihash_header_len,
	const char* nonce,
	unsigned int nonce_len,
	std::function<bool()> cancelf,
	std::function<bool(const std::vector<uint32_t>&, size_t, const unsigned char*)> solutionf,
	std::function<void(void)> hashdonef,
	CPU_TROMP& device_context)
{
	equi eq(1);
	eq.setnonce(tequihash_header, tequihash_header_len, nonce, nonce_len);
	eq.digit0(0);
	eq.xfull = eq.bfull = eq.hfull = 0;
	u32 r = 1;

	for (; r < WK; r++) {
		if (cancelf()) return false;
		r & 1 ? eq.digitodd(r, 0) : eq.digiteven(r, 0);
		eq.xfull = eq.bfull = eq.hfull = 0;
	}

	if (cancelf()) return false;

	eq.digitK(0);

	for (unsigned s = 0; s < eq.nsols; s++)
	{
		std::vector<uint32_t> index_vector(PROOFSIZE);
		for (u32 i = 0; i < PROOFSIZE; i++) {
			index_vector[i] = eq.sols[s][i];
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
