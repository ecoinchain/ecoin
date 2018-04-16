#include <chainparams.h>
#include <coins.h>
#include <consensus/consensus.h>
#include <consensus/merkle.h>
#include <consensus/tx_verify.h>
#include <consensus/validation.h>
#include "crypto/equihash.h"
#include <validation.h>
#include <miner.h>
#include <policy/policy.h>
#include <pubkey.h>
#include <script/standard.h>
#include <txmempool.h>
#include <uint256.h>
#include <util.h>
#include <utilstrencodings.h>
#include <amount.h>

#include <test/test_bitcoin.h>

#include <memory>

#include <boost/test/unit_test.hpp>

struct GenesisMainTestingSetup : public BasicTestingSetup {
    GenesisMainTestingSetup() : BasicTestingSetup(CBaseChainParams::MAIN) {}
};

BOOST_FIXTURE_TEST_SUITE(genesis_main_tests, GenesisMainTestingSetup)

BOOST_AUTO_TEST_CASE(GenesisMain)
{
    // 这样多个线程进行计算的时候genesis可以不同
    int64_t nTime = 1523757600 + GetRand(28800);
    printf("nTime = %lld\n", nTime);
    CBlock genesis = CChainParams::CreateGenesisBlock(
            nTime,
            uint256S("0x0"),
            ParseHex(""),
            0x1f07ffff, 1, GENESIS_MONEY);
    CBlock *pblock = &genesis;
    const CChainParams params = Params();

    unsigned int n = params.EquihashN();
    unsigned int k = params.EquihashK();

printf("n = %d, k = %d\n", n, k);
    // Hash state
    crypto_generichash_blake2b_state state;
    EhInitialiseState(n, k, state);

    // I = the block header minus nonce and solution.
    CEquihashInput I{*pblock};
    // I||V
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << I;
    // ss << pblock->nNonce;

    // H(I||V||...
    crypto_generichash_blake2b_update(&state, (unsigned char*)&ss[0], ss.size());

    while (true) {
            // Yes, there is a chance every nonce could fail to satisfy the -regtest
            // target -- 1 in 2^(2^256). That ain't gonna happen
            pblock->nNonce = ArithToUint256(UintToArith256(pblock->nNonce) + 1);
            printf("nonce = %s\n", pblock->nNonce.GetHex().c_str());

            // H(I||V||...
            crypto_generichash_blake2b_state curr_state;
            curr_state = state;
            crypto_generichash_blake2b_update(&curr_state,
                                              pblock->nNonce.begin(),
                                              pblock->nNonce.size());

            // (x_1, x_2, ...) = A(I, V, n, k)
            std::function<bool(std::vector<unsigned char>)> validBlock =
                    [&pblock](std::vector<unsigned char> soln) {
                printf("soln = %s\n", HexStr(soln).c_str());        
                pblock->nSolution = soln;
                printf("blockHash = %s\n", pblock->GetHash().ToString().c_str());
                return CheckProofOfWork(pblock->GetHash(), pblock->nBits, Params().GetConsensus());
            };
            bool found = EhBasicSolveUncancellable(n, k, curr_state, validBlock);
            if (found) {
                goto endloop;
            }
        }
endloop:
    printf("pblock nonce = %s\n", pblock->nNonce.GetHex().c_str());

    crypto_generichash_blake2b_update(&state,
                                              pblock->nNonce.begin(),
                                              pblock->nNonce.size());

    // pblock->nSolution = ParseHex("07c0fa1e314ca6a95c1d604a3a54858922c52e52984d05f496217068f35d90c8e5469b63");
    bool isValid;
    printf("nTime = %lld, pblock soln = %s\n", nTime, HexStr(pblock->nSolution).c_str());
    EhIsValidSolution(n, k, state, pblock->nSolution, isValid);
    printf("nTime = %lld, isValid = %s\n", nTime, isValid ? "true" : "false");
    printf("nTime = %lld, blockHash = %s\n", nTime, pblock->GetHash().ToString().c_str());
}

BOOST_AUTO_TEST_SUITE_END()