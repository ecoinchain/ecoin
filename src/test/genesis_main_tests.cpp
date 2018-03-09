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

#include <test/test_bitcoin.h>

#include <memory>

#include <boost/test/unit_test.hpp>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nSolution = nSolution;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nSolution, nBits, nVersion, genesisReward);
}

struct GenesisTestingSetup : public BasicTestingSetup {
    GenesisTestingSetup() : BasicTestingSetup(CBaseChainParams::MAIN) {}
};

BOOST_FIXTURE_TEST_SUITE(genesis_main_tests, GenesisTestingSetup)

BOOST_AUTO_TEST_CASE(GenesisMain)
{
    CBlock genesis = CreateGenesisBlock(
            1231006505,
            uint256S("0x0"),
            ParseHex(""),
            0x1f07ffff, 1, 50 * COIN);
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
    printf("pblock soln = %s\n", HexStr(pblock->nSolution).c_str());
    EhIsValidSolution(n, k, state, pblock->nSolution, isValid);
    printf("isValid = %s\n", isValid ? "true" : "false");
    printf("blockHash = %s\n", pblock->GetHash().ToString().c_str());
}

BOOST_AUTO_TEST_SUITE_END()