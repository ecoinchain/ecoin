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
    // bip34要求height放在coinbase输入脚本的首位.
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
    const CScript genesisOutputScript = CScript() << OP_DUP << OP_HASH160 << ParseHex("3bf1b57c1b21c5eb32caf9e1f0c6ff795156dd20") << OP_EQUALVERIFY << OP_CHECKSIG;
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
        consensus.nSubsidyHalvingInterval = 2000000;
        consensus.powLimit = uint256S("007fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2 * 24 * 60 * 60; // two days
        consensus.nPowTargetSpacing = 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 2736; // 95% of nMinerConfirmationWindow
        consensus.nMinerConfirmationWindow = 2880; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // The best chain should have at least this much work.
        // todo 后面等链变长之后需要修改
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        // todo 等到链成长到一定的长度再设置.
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
        const size_t N = 200, K = 9;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        // 网络部署时间跟genesis设置的时间不能超过1天
        genesis = CreateGenesisBlock(
            1523879995,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000001"),
            ParseHex("000bc5a16ae6484b82ede034709093525612d81db71528856e056b87ab9dd0643fcbc75ca26436196c7f145d7aae275df5b3e4e194ad277be131bb1330520d28c64fed9d224adf79df63831734ca9b6cc83ccae71865f7cf512ad327dd2ab1eaa9fa8cd12ce89dcf7d35b5c305a81af131005f03c6c5b7e1e9f745b790dc218b22e28d9cfdc31dff097e8554be06c45477cc0c496b3534fd599936d77565db3235924d7b22af1536054ecbfca5976c27af9b81ea5ea8aa45570c375f321d4b87b0f4acf2c372f3239c876270a23ef573ada523d751dcc45f08d7245ce459ffb07e2f20ca1d0b572a819576a0e5b2a1f4f6f3595344cfe8daa5778576077af095e0a43ec35359e22c602716609fafa6be57231bd230bfe576db810ba460d6bfba11ba0a0f3bad121bfcaa991eff2dcbc6b322413b616e0dcb98a856263bd4f91113f2535bfbf2b4a3b61d6542ba6bbfce01a3f6d97493f119b0d0e12f9ffebcf9646736df630a74f2c7bb2496d7d405752aff62515e7578f4eb0404512cfccb1624e9f45dd33875b3558d74a515c62e18e5fb605748ef85215d536497a8847d2f704a4fca024cd73a57dbdf211320b4f8975d9162cf675b01d33c966bd2a1943d4d155b7471e9e7224208939958b7038a7623b9373f5de595d6962156735e95ef3b46ec0b19acc88ac3eccf82e3a3969d6e031676aff3d0ad078ac74648968121ccfb22c0ff52be8d3714391bf7389aed8925701f87cd9db68e2fb6e1166ab05c1dd11266b46d45901475696ef41fc7f5198aab3019707625e6c37b690beb8f9c9f586f6bf7d5d22546bbfda31f7834b83f5c9731750432d3d0cb8929c429b416b43d64f55192f5d38fe43b671f03df0cabb0933dfc4e2e57d17c0291d4d71577c408cbe3824967814d98cc320c9a3f26fc69ddedc0f3de0b6291ba9cedd762f200a9bd055be032dfec53d2ad6e189552048c592fc52257331d06cff1ad5594cb8fc0785c375928ffbbd30aa48fd38d1fdc2b54b4a0d0ce6dfac63e927973ec11e1b5bfe199d1c31ae151a0f4baff7594598d8c0023afe7d3ab1d3d7f121e99d6e7544bfe960719ab7538ca7a073597f42702a0e532c55f4ea6548094c84c39e59263174e9f956e25b51ad3db3d2e0a2811cc444ceb2bd89722687b613b9a7fa6d9644fdda43fed9f053d8f235ce9ed677de8761cf3e4a6d18db0b237d60f43c918fa6d36f573d285785cb6f19a1324d349d91e6c46a0a3f872f3e8b803143d1f324a160bb4dce62fd259be14614bbbc13ee50a94d81a6d4af45ad6a605810c4bf55aa71d0e61b3164ef9e93336d1fdd38d35b51daa70dee90bdc7f367234e5ec52403c99ee9908a3bed7d1c73d74cd76d1b2e1e666fd83b2d2d538607c0e37e9ea2509dffac9c7fdf47986dca4375dc9012d6f2990dd6697929e91ade7ad273d919d6ec6980bb48a0e91dd77d3a11993edaac897e57a55db920c08d014499660ff4ff4d251a619df9fd1a319ba9fc514f63bc4a445f414659a84b81cca30d592dbd31eab0c475aa11b991eb1d5de81fa80f9c5ce315ef6756716e6c470142738abe977850006e76331b9c4f9769a122214580f90806edb5692d36f190d156ccbdfbe6d167dd6ffbcf5f957d40304d26e4bf4f571729de42c0311407bbf06e65156f1826aa3b3faedd4ed5a500b0c869dcb4ec822637a9000f90190e88a42ff3a8f9432d5cbeb44d23551109373f4d143101d4aa91db3c043a20fd9abe267a9a1f285161039611aadcf36e30b0441be899f58b509f16c23c09e78cc55cf7f930c473c3f9fb5b5123cd76d4015d731d6c5cdd3b2d1cc77196c52e3764c2cab4816d22b556d7388bdd9f863a11a28748fd55a8a87fc35e953c3579feeada87a17ce"),
            0x1f7fffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x007efb2d4b4006d02f595d8d95e4c5be9815baf43100caa4266229d270abff4b"));
        assert(genesis.hashMerkleRoot == uint256S("0x031fcb90377f5fdae730e3689f36d032cbcc036c27a935bf56a08ed958da8b2e"));

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as a oneshot if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
        // 暂时没有seed
        // vSeeds.emplace_back("59.110.153.149"); // Pieter Wuille, only supports x1, x5, x9, and xd
        // vSeeds.emplace_back("60.205.188.43"); // Matt Corallo, only supports x9
        // vSeeds.emplace_back("47.97.121.28"); // Luke Dashjr
        // vSeeds.emplace_back("47.97.201.211"); // Christian Decker, supports x1 - xf
        // vSeeds.emplace_back("seed.bitcoin.jonasschnelli.ch"); // Jonas Schnelli, only supports x1, x5, x9, and xd
        // vSeeds.emplace_back("seed.btc.petertodd.org"); // Peter Todd, only supports x1, x5, x9, and xd

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,33); // E为地址的第一个字母.
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,15); // 7为地址的第一个字母.
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x14, 0x03, 0x12, 0x42};
        base58Prefixes[EXT_SECRET_KEY] = {0x43, 0x5d, 0x8c, 0xb2};

        bech32_hrp = "ec";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        // todo 区块链的检查点，等到链成长到一定程度再设置.
        checkpointData = {
            {
                {0, uint256S("007efb2d4b4006d02f595d8d95e4c5be9815baf43100caa4266229d270abff4b")},
            }
        };

        // todo 区块链交易信息，等到链成长到一定程度再设置.
        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        /* 在链的初始阶段主网允许fallback fee*/
        m_fallback_fee_enabled = true;
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 2000000;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2 * 24 * 60 * 60; // two days
        consensus.nPowTargetSpacing = 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 2160; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2880; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // The best chain should have at least this much work.
        // todo 后面等链变长之后需要修改.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        // todo 等到链成长到一定的长度再设置.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"); //1135275

        // 用可见的消息magic感觉更好点，ecot表示ecoin test net
        pchMessageStart[0] = 0x45; // e
        pchMessageStart[1] = 0x43; // c
        pchMessageStart[2] = 0x4f; // o
        pchMessageStart[3] = 0x54; // t
        nDefaultPort = 18877;
        nPruneAfterHeight = 1000;
        const size_t N = 200, K = 9;  // Same as mainchain.
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        // 网络部署时间跟genesis设置的时间不能超过1天.
        genesis = CreateGenesisBlock(
            1523876400,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000001"),
            ParseHex("0046ebc0ca4a720fa622b1307cd81ab9014eaed69a0af0075b82242a9176bfe42a43419f51707d180f28307e9ee718e8c595be8b2468612c983aa3f8db893131a8976e7d92a133fcfc9649887eb5d9bbe2b1de36037514e40b43fe831e0e20ae087266b523bb0a32df076cf9df72df722dd45399397c5ce16285f83bd31d123c41106849536f30f3d1ab23136ca612053f98fb41ba9d4a36582c87f83e867ee341a0e1c42cf38c960323dd21c68309c4aa2d012f44411551ca537dc2c410ec75395fc71c0c935764f997fa0f89c137cf51961f77ce710e206741f178533bff2a5f35f017f673092d669df48f1e9c337de2b5458b4fc196208293c7860c8ad1717806032e51690b9f9fec7a038e753f4e1225b1c5aff1ce8465aedf46a87a5dd0c672837c7eb30cdad6357ed98c93b1f4713e9c9dfdbd108bf923052cd152dce65998d34a4e73083dedc9cde257b2f33a01893912be15a167e9a274ec3a7a2c65d2945b7e05210d156e6912e8e9c8c5c599f152d99ad983bfca4107f67c32bcdcfe93548f1212a790aa70d6bab47ce85c0c0b6720b5a77bbb569a31855a686b15c599f7d80230eeb09e19da4af12350d8c6670ac0663e2787227174158447351c9fc0ae590ff8e94bda48833da52c1f6b5255fa1643fdd23bc5dbe9d0cd6633a7d49ecf25654149868b5c59c4c285b970db39a2800efa9cf7093502731d1eeaf15b59a15771db8f38a960c563f079646ed098e2a95fb4f49869f1793896e2c43b0d0b10205b9127560a851fd0e40fb82cd5bd10f1ed8fc223857deb6d52dfeaef5ff2e7959d12b23188f941b80a2ce914882bdcf3edf56172309b07455fc1f770e70a7b6c778167d345f389c1a49aa92146da931ca3c828d582686bee3c11e872b341faa8160993a9dd38c64638065bd6a5a7eb4c70d470c7d04fea34f61c01bf00a59828d71d65b72f3353b87d230893827d7dcc9427e252ce03a1e5359d6e45d3f54bd88ee2fa387a250f9af08c6d8dadfe8f14143577a53d05201a2bd9b932c4f48d3df5010bf67be988356c37e6dbab56f4fd070d965850427c92997984683b38e29d81318dd1cb20b56ceed1d3ce6773d02283ebbd6c30d267bbfc3817b9e47c5b46a013d63d04793fd7118ad80c5a1d2f24fee2b065a840679a273361157766aa26ed72e8b803c6630ae9c9be1114c20184dbb293ad3a38d27608487b9f7ee830844ff6f6b497882709398662555ee437f77ce58e530403485284dc1d6947bac0481f040245b69f2b25a3a7d1779a65581a2dda5a16f458249b03ff6f6980c3077642e2e1ffc5c8a36968c8ade5c0173ac5a31f4c2949d226d6da21eb4cadf731f0f9f30d5d6081305d9b373e9603f4f7af66be2792fa4a85165e06f632dc1d8559b0216fa91b2a38f2ee6bebd004e44102a10b0e8b2c25928885728cf14be43e3e0a290e349863107f33c0f1a3beb3b028ab54d0df99fb0bbb68af430acf7c93a338f0abdf663ed388b941c94389cb6461a3714b96cd04e05cf982e3be0fff518705162babee503436f697e42cf8362dd608ccb452cd40697f382a548cfd679c29dd00e23fef38945d72170bf92e797344419f7875c6cd9236e655e9b7b329d82dd20e1d1a4cb3453b4fc61591dd172ec104ffec69078f2b7f24deb997358e16c0b4d19afde822940e890e35f5b878156e24d3ea32de34bca2a551d2fa1b6f2e9f0a15d30c67ff48f5092ac5e351aaccf6b8248e47edac4a2b63e44fa2a175bcd6f09331f2127ae1d80b8fb538dd0ec62b578190cf4cddbf150c8995174e1977de525cdc10e3c225b580956303e5acde5126771e0bdda48ed7ad2cc699625e7c67a80af7c8f9b15b3616ea6d766383d9c406f39edbbb1fe9d71cf086fe"),
            0x207fffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x09bf3167d39207977109a5864c4406cf5de8597f66ac520eafb92e93caf67b5a"));
        assert(genesis.hashMerkleRoot == uint256S("0x031fcb90377f5fdae730e3689f36d032cbcc036c27a935bf56a08ed958da8b2e"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        // 暂时没有seed
        // vSeeds.emplace_back("testnet-seed.bitcoin.jonasschnelli.ch");
        // vSeeds.emplace_back("seed.tbtc.petertodd.org");
        // vSeeds.emplace_back("seed.testnet.bitcoin.sprovoost.nl");
        // vSeeds.emplace_back("testnet-seed.bluematt.me"); // Just a static list of stable node(s), only supports x9

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x12, 0x93, 0xff, 0x3a};
        base58Prefixes[EXT_SECRET_KEY] = {0x3e, 0x89, 0xde, 0x01};

        bech32_hrp = "te";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        // todo 区块链的检查点，等到链成长到一定程度再设置.
        checkpointData = {
            {
                {0, uint256S("09bf3167d39207977109a5864c4406cf5de8597f66ac520eafb92e93caf67b5a")},
            }
        };

        // todo 区块链交易信息，等到链成长到一定程度再设置.
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
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000004"),
            ParseHex("1620db1dd1869b8fe92f99dc5443dcc727ff299e189827275ee9f931529a36e37b8ce927"),
            0x207fffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x34eb897a5f8e37c156c4170becc5993740003cbb3ce4a326ddd31701d3a749d0"));
        assert(genesis.hashMerkleRoot == uint256S("0x031fcb90377f5fdae730e3689f36d032cbcc036c27a935bf56a08ed958da8b2e"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("34eb897a5f8e37c156c4170becc5993740003cbb3ce4a326ddd31701d3a749d0")},
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
        base58Prefixes[EXT_PUBLIC_KEY] = {0x12, 0x93, 0xff, 0x3a};
        base58Prefixes[EXT_SECRET_KEY] = {0x3e, 0x89, 0xde, 0x01};

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
