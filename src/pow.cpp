// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>

#include <boost/multiprecision/cpp_int.hpp>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <chainparams.h>
#include <crypto/equihash.h>
#include <streams.h>
#include <util.h>

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

static boost::multiprecision::uint512_t decode_compact(uint32_t nCompact)
{
	/**
	* The "compact" format is a representation of a whole
	* number N using an unsigned 32bit number similar to a
	* floating point format.
	* The most significant 8 bits are the unsigned exponent of base 256.
	* This exponent can be thought of as "number of bytes of N".
	* The lower 23 bits are the mantissa.
	* Bit number 24 (0x800000) represents the sign of N.
	* N = (-1^sign) * mantissa * 256^(exponent-3)
	*
	* Satoshi's original implementation used BN_bn2mpi() and BN_mpi2bn().
	* MPI uses the most significant bit of the first byte as sign.
	* Thus 0x1234560000 is compact (0x05123456)
	* and  0xc0de000000 is compact (0x0600c0de)
	*
	* Bitcoin only uses this "compact" format for encoding difficulty
	* targets, which are unsigned 256bit quantities.  Thus, all the
	* complexities of the sign bit and using base 256 are probably an
	* implementation accident.
	*/

	boost::multiprecision::uint512_t result_big_number;

	int nSize = nCompact >> 24;
	uint32_t nWord = nCompact & 0x007fffff;
	if (nSize <= 3) {
		nWord >>= 8 * (3 - nSize);
		result_big_number = nWord;
	}
	else {
		result_big_number = nWord;
		result_big_number  <<= 8 * (nSize - 3);
	}

	return result_big_number;
}

static boost::multiprecision::uint512_t UintToCpp512(const uint256 & n)
{
	std::string n_string = n.ToString();
	return boost::multiprecision::uint512_t("0x" + n_string);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    auto bnPowLimit = UintToCpp512(params.powLimit);
//    arith_uint512 bnNew;
	boost::multiprecision::uint512_t bnNew = decode_compact(pindexLast->nBits);

//	bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

	std::stringstream converted_stream;
	converted_stream << std::hex << std::showbase << bnNew;

	std::string converted_string = converted_stream.str();
	return UintToArith256(uint256S(converted_string)).GetCompact();
}

bool CheckEquihashSolution(const CBlockHeader *pblock, const CChainParams& params)
{
    unsigned int n = params.EquihashN();
    unsigned int k = params.EquihashK();

    // Hash state
    crypto_generichash_blake2b_state state;
    EhInitialiseState(n, k, state);

    // I = the block header minus nonce and solution.
    CEquihashInput I{*pblock};
    // I||V
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << I;
    ss << pblock->nNonce;

    // H(I||V||...
    crypto_generichash_blake2b_update(&state, (unsigned char*)&ss[0], ss.size());

    bool isValid;
    EhIsValidSolution(n, k, state, pblock->nSolution, isValid);
    if (!isValid)
        return error("CheckEquihashSolution(): invalid solution");

    return true;
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
