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
#include <ed25519/ed25519.h>
#include "validation.h"

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (pindexLast->nHeight < 5580) {
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
        } else {
            // Return the last non-special-min-difficulty-rules-block
            const CBlockIndex* pindex = pindexLast;
            while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                pindex = pindex->pprev;
            return pindex->nBits;
        }
    }

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
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

	unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    arith_uint256 bnNewtmp;

    if (pindexLast->nHeight > 576)
	{
        const CBlockIndex* pindex = pindexLast;
        while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
            pindex = pindex->pprev;
        bnNewtmp.SetCompact(pindex->nBits);
    }
	else
	{
        bnNewtmp.SetCompact(pindexLast->nBits);
    }

	boost::multiprecision::uint512_t bnNew = UintToCpp512(ArithToUint256(bnNewtmp));

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
    blake2b_state state;
    EhInitialiseState(n, k, state);

    // I = the block header minus nonce and solution.
    CEquihashInput I{*pblock};
    // I||V
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << I;
    ss << pblock->nNonce;

    // H(I||V||...
    blake2b_update(&state, (unsigned char*)&ss[0], ss.size());

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

bool CheckAuthorization(const CBlock *pblock, const CChainParams& params)
{
    CBlockIndex * chainIndex = chainActive.Tip();
    if (chainIndex == nullptr ||
            params.GetConsensus().authorizationForkHeight < 0 ||
            chainIndex->nHeight < params.GetConsensus().authorizationForkHeight) {
        return true;
    }
    if (!params.GetConsensus().authorizationKey.IsFullyValid()) {
        return true;
    }

    if (pblock->vtx.empty() || !pblock->vtx[0]->IsCoinBase()) {
        return false;
    }

    const CTransaction& coinbase = *pblock->vtx[0];
    CScript scriptSig = coinbase.vin[0].scriptSig;
    // 0x40 + 64个字节的signature
    if (scriptSig.size() < 65) {
        return false;
    }
    CScript::const_iterator pc = scriptSig.begin();
    std::vector<unsigned char> sig;
    opcodetype opcode;
    if (!scriptSig.GetOp(pc, opcode, sig)) {
        return false;
    }
    // signature长度是64
    if (opcode != 64) {
        return false;
    }
    CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
    for(auto vout : coinbase.vout) {
        ss << vout;
    }
    auto hash = ss.GetHash();
    return params.GetConsensus().authorizationKey.Verify(hash, sig);
}
