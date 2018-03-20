// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>

#include <assert.h>

#include <chainparamsseeds.h>

// For equihash_parameters_acceptable.
#include <crypto/equihash.h>
#include <net.h>
#include <validation.h>
#include <amount.h>

#define equihash_parameters_acceptable(N, K) \
    ((CBlockHeader::HEADER_SIZE + equihash_solution_size(N, K))*MAX_HEADERS_RESULTS < \
     MAX_PROTOCOL_MESSAGE_LENGTH-1000)

CBlock CChainParams::CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    // bip34要求height放在coinbase输入脚本的首位
    txNew.vin[0].scriptSig = CScript() << 0 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
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

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
CBlock CChainParams::CreateGenesisBlock(uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    const CScript genesisOutputScript = CScript() << OP_DUP << OP_HASH160 << ParseHex("b95a9a41ec86fd7908986b903b5c2a8f549e6ea3") << OP_EQUALVERIFY << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nSolution, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 210000;
        // todo 临时使用reg test的参数
        // consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // The best chain should have at least this much work.
        // todo 后面等链变长之后需要修改
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        // todo 等到链成长到一定的长度再设置
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"); //506067

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        // 用可见的消息magic感觉更好点，ecom表示ecoin main net
        pchMessageStart[0] = 0x45; // e
        pchMessageStart[1] = 0x43; // c
        pchMessageStart[2] = 0x4f; // o
        pchMessageStart[3] = 0x4d; // m
        nDefaultPort = 8877;
        nPruneAfterHeight = 100000;
        // const size_t N = 200, K = 9;
        // todo 临时使用reg test的参数
        const size_t N = 48, K = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        // genesis = CreateGenesisBlock(
        //     1231006505,
        //     uint256S("0x0"),
        //     ParseHex(""),
        //     0x1f07ffff, 1, GENESIS_MONEY);
        // todo 临时使用reg test的参数，main的参数计算一个合适的nonce耗时太长了
        // 网络部署时间跟genesis设置的时间不能超过1
        genesis = CreateGenesisBlock(
            1521474869,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000002"),
            ParseHex("05fa8a47319cc4ace01b52c7764281e1db9e072303cff096e097a618af6e3de28ba5d76a"),
            0x207fffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        // printf("consensus.hashGenesisBlock = %s\n", consensus.hashGenesisBlock.ToString().c_str());
        // printf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
        assert(consensus.hashGenesisBlock == uint256S("0x214033fec623e9d1224e20d5ce2b24906585b9fb80d515916792608bf0c4190a"));
        assert(genesis.hashMerkleRoot == uint256S("0x0860d3e7c114c3cc2daa492ce974006ea74c2e4197320e393889a236c8191445"));

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as a oneshot if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
        // seed目前是不启用的
        // vSeeds.emplace_back("59.110.153.149"); // Pieter Wuille, only supports x1, x5, x9, and xd
        // vSeeds.emplace_back("60.205.188.43"); // Matt Corallo, only supports x9
        // vSeeds.emplace_back("47.97.121.28"); // Luke Dashjr
        // vSeeds.emplace_back("47.97.201.211"); // Christian Decker, supports x1 - xf
        // vSeeds.emplace_back("seed.bitcoin.jonasschnelli.ch"); // Jonas Schnelli, only supports x1, x5, x9, and xd
        // vSeeds.emplace_back("seed.btc.petertodd.org"); // Peter Todd, only supports x1, x5, x9, and xd

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,33); // E为地址的第一个字母
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,15); // 7为地址的第一个字母
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "ec";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        // todo 区块链的检查点，等到链成长到一定程度再设置
        checkpointData = {
            {
                {0, uint256S("214033fec623e9d1224e20d5ce2b24906585b9fb80d515916792608bf0c4190a")},
            }
        };

        // todo 区块链交易信息，等到链成长到一定程度再设置
        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        /* disable fallback fee on mainnet */
        m_fallback_fee_enabled = false;
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 50000;
        // todo 临时使用reg test的参数
        // consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2 * 24 * 60 * 60; // two days
        consensus.nPowTargetSpacing = 4 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 720; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // The best chain should have at least this much work.
        // todo 后面等链变长之后需要修改
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        // todo 等到链成长到一定的长度再设置
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"); //1135275

        // 用可见的消息magic感觉更好点，ecot表示ecoin test net
        pchMessageStart[0] = 0x45; // e
        pchMessageStart[1] = 0x43; // c
        pchMessageStart[2] = 0x4f; // o
        pchMessageStart[3] = 0x54; // t
        nDefaultPort = 18877;
        nPruneAfterHeight = 1000;
        // const size_t N = 200, K = 9;  // Same as mainchain.
        // todo 临时使用reg test的参数
        const size_t N = 48, K = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        // genesis = CreateGenesisBlock(
        //     1296688602,
        //     uint256S("0x000000000000000000000000000000000000000000000000000000000000000b"),
        //     ParseHex("00d276f2e04b5b4b2a8df7dd7fe5a262fc353e423c1ac4c16e421a12218f517380237b3da270e09397a20aca40e1e9df21a91ad3b62eb437968dd76730c983132aac7cc9c66f6968ae77b1fbd5d78a196f7420e9027bc3fc9264dad9becf034beb4958ccfd6774eb141aefcd82a59a6118ef5e874f6f63fcc1eea8d62b270f57fbdcff8db199fe13e4b6d6cfeb2ad99299d3a33b47530aa792729adabbc59b25d7164348093b3b9301313a888a94fbb70d7cc128972aced4efff19930a064ec67590077d6d8d65548e9376b48fae09be1ad206ee5ce32702ad382a74e2549a97c93653bf3b8edf1a33cdf9ba91be45dbbf1285327831999c6bdc17e1056f6ca212673525dba130d41bfdaa8af254bd3d510b19b3d6178b45cc8535956cacf3d1e188604e52ba15eb859e092d2ef9cdd9f160f4aebe208bb00e4dd333099ade1f9b164f7c40f4022a46e8fd4b1079ff3600e20aa42e123f3cdb70d398e0ea402645bc5f75ea07fbeab42451029f37d8f4dae3bac336f6245a25160f183feb03b0bfb5a5e4b6935f50fb8302729a79f02b19d42d1fab528763ddd46c8ac8af966701b778d70359563b61d95009f9d072b8b92566b15c4b5d43b006be4a26b9b03f7ff582f14a4cbb95a96633f7a6ce286ccb3e6ffabd99e50c743a5d5bacd296c8f782f8397b046c8b9b55d5a778740b8ab48cd5acf8d5218404d2794b60200a7f42302185f3aa29b5d7ab5912f33395ef812612f77b5c74245a7a7eea718cf44e84da0aa22ab369c9bfeb8cf851195b24f618eab0cfc14a163cbd4df2fac01dff61d2b4b9787e83a28f9e60ef062a78f83fd88a25f803f1aebeb1df2120b85366440ae6877c160d1eaec4c353bf9e2928e1139a8f435008750a53534dadacd87fb31398d6bd793453ea75473b1cfbb72d5b09d9dc640c589e6a49072c3f7f547601d57a54eb1349a7abf767f8a6f50a8aa0c9d8e9043d9bbd29426e64a5985856d69cd667f610a09d96081016b6a946510626bcc613f743e7064d4c2559eac51b6f61f881091ac36b1112b57f59e5b29268bf153305ecef08a4d6697ae1b80207bdf63b6d9a74ed0f5f287e2b05ab0a74b7be2fe5898be9ad09e07cdcea8b07015eb9f54bffeee745b16d30324eb9d05cdcbb7a1375a6211b9601a18157559e51f51779e3a0b7cd3d0484268e6d5747bfd3b983d7b9abfe8b60cfbe38fa0484fd02c78465fb222272912da21c7ef31c7ba67b0794b1ce74a0428ba8ecfc88e775876f5e135d6221244397d12c8ee82735ef59fc26ddf366953d5d7c2304b35d726b84b9694d2d5446b22aa56eebb4f776511cbf8fa7c4a7fa0370570215055ca8722e1edbb142061cc528db5e7233a26db10bc29b4910ed4558ba6f0af97b503898d04f8248c5bac3ca587e0652194b1c070e73aec1098f7fb930a63fccebb8860d35bc0192075b931949e5334df6a413fa2d270d3e40cabe87e6098e4ad9d54b7467321893113dc3f308dfb319a0fa317a99db89a48fe1d201a433f5a7e20ee3e03d94a811d21aac5b905bb5e77eb22c1ad1dbc26e9e3470b85d09bdd170a8bc3d6c4a2680f66fe15dac163283f230aa87b06c26afed9160e81cb7f8432aea8377e99846e8b6f212364807e6db27073b4027be4e82927af207882e8109d3486bbbd8b678d85a6e4a8f9ed590de430b5fa20a111539e87cc44d7fcf83394404d9b61f17fb34c534e4e89382c9d2770ce197fecb737ab4d4433cf602d554e54bac0a14bd17608d4db69f33ab7c08a7d607366e99f1a0c1a4cf08dfaed97f1b9870174871974f43b6fc1fd73f3637155349d2af14babcf30f6d7ce2bfd228972ec7d38922bfed7f38b19fe16a279bdc936a8c5a378f90f7165f7652da180494681e"),
        //     0x2007ffff, 1, GENESIS_MONEY);
        // todo 临时使用reg test的参数，test的参数计算一个合适的nonce耗时太长了
        // 网络部署时间跟genesis设置的时间不能超过1
        genesis = CreateGenesisBlock(
            1521474869,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000001"),
            ParseHex("0155d0db683c4aab6b1e18279a54271932ff0659cf09432d2d374e0fcb88b6a25e5183cd"),
            0x207fffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        // printf("consensus.hashGenesisBlock = %s\n", consensus.hashGenesisBlock.ToString().c_str());
        // printf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
        assert(consensus.hashGenesisBlock == uint256S("0x3fa13a55a63059b64a508c18c2980af5c7d5d0d8de054cf3be72330eb03964f2"));
        assert(genesis.hashMerkleRoot == uint256S("0xb56e77729e6d50fbb3cf2a47d08edf2ae1a239945d626f870e6dbb43ae32db65"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        // seed目前是不启用的
        // vSeeds.emplace_back("testnet-seed.bitcoin.jonasschnelli.ch");
        // vSeeds.emplace_back("seed.tbtc.petertodd.org");
        // vSeeds.emplace_back("seed.testnet.bitcoin.sprovoost.nl");
        // vSeeds.emplace_back("testnet-seed.bluematt.me"); // Just a static list of stable node(s), only supports x9

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "te";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        // todo 区块链的检查点，等到链成长到一定程度再设置
        checkpointData = {
            {
                {0, uint256S("3fa13a55a63059b64a508c18c2980af5c7d5d0d8de054cf3be72330eb03964f2")},
            }
        };

        // todo 区块链交易信息，等到链成长到一定程度再设置
        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        /* enable fallback fee on testnet */
        m_fallback_fee_enabled = true;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        // 用可见的消息magic感觉更好点，ecor表示ecoin reg net
        pchMessageStart[0] = 0x45; // e
        pchMessageStart[1] = 0x43; // c
        pchMessageStart[2] = 0x4f; // o
        pchMessageStart[3] = 0x52; // r
        nDefaultPort = 18777;
        nPruneAfterHeight = 1000;
        const size_t N = 48, K = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        genesis = CreateGenesisBlock(
            1296688602,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000002"),
            ParseHex("05fa8a47319cc4ace01b52c7764281e1db9e072303cff096e097a618af6e3de28ba5d76a"),
            0x207fffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        // printf("consensus.hashGenesisBlock = %s\n", consensus.hashGenesisBlock.ToString().c_str());
        // printf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
        assert(consensus.hashGenesisBlock == uint256S("0x214033fec623e9d1224e20d5ce2b24906585b9fb80d515916792608bf0c4190a"));
        assert(genesis.hashMerkleRoot == uint256S("0x0860d3e7c114c3cc2daa492ce974006ea74c2e4197320e393889a236c8191445"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("214033fec623e9d1224e20d5ce2b24906585b9fb80d515916792608bf0c4190a")},
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "ecrt";

        /* enable fallback fee on regtest */
        m_fallback_fee_enabled = true;
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
