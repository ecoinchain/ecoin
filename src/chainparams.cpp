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

        // 网络部署时间跟genesis设置的时间不能超过1天.
        genesis = CreateGenesisBlock(
            1524206068,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000006"),
            ParseHex("00c9c1cf397445d1bf72a2db19b821399cef9517e710b09ca26f091fbef2bfe25ecbf3948a451a7801ed0cd0e1cdd6782c37f26d6179a1df97a4e37456b83621a3e5df7f29a48fe26f128569ea4750b02338fe430b04eaec448dede3a46d95327e699b4185fef82505466a4615fb5895c7235ff7ce3c5ea3b3a8191fd9030fa307cbe2a16849f320d700db4ba45f49e19fb9e010b18da64aa1ee75f59253410d4886bb09fbfc482806c4788e5a678df5fb09c5355dd178deacda9e397a293b2b5b2e910a015663e3cc9c4cfae9e0d297e30c06dce1166274c1e3f43673f8234342dd41b30b2c1f13a56e3eebab4091631aa3d57a7f823f0bebde4aea08c902ca866de87bede803fb75f900568f72956e5421230e84535e8f671a98223827de0c91912af91038169d54571a3a0dd1ff9b63695328dbeb95ac3d71dc3670fc54a862767bb854a6cf556a93ff1682dc129d0196118ecc97f606e2d573880971f25dfbe957766e43ffe5a30832d4b3cccdeab893faf1a2f4961d3b9902a09789e78198828246a23488e0e9f0bfa9d0474403b74c12446a28ebedce67cf3c7655de2858d1623a03f0b8357593457f65abe224b77a1904e2f79c953d51d49bb21a6b6c7db330c851b4fe0bba14e3969d8b20078ba08a0c293ef7b753fcc6f15b4edfe7f9f5c827d5cd329e358eb3ee8fa4fd6d3d6825d51d7a02c10aa162958f17491adaf1d324e371c07d83603e120317a916557a53c6cda524826df2ee9dddfc80bcc03913122774d94673d648c5b330a7227c65840073af7b2c43ca81915ce9cb52d117b4124b5c9a84b874df74122d22b6609610c112bfb4f1f2330401e55374b297230f66659556e831227595beeceeb32dd4ab54ab5f334a4cb79a6f504b8f94958d95d575f18b2472d9cb37b2e2eb5d66a0393f882879114ade5f13381917fd01b28bd61313b079ac47dd3594e9ea3f68211c2cc90e76e2d35d70f5a9c10a234684508966be57d91e051c4fecc4398ece0df90155c7a0e6cc2dfcf6d5038f36dfe43103718dd1ceac53716cda69c1b49af422700ad555b8af1eb1677bc8b189c4940714b26b877e204984b55c2fe6fe4739d1f9963653966e6db63f396e2880cbac3996eb88d80ec2e59b65c119be827ca98a44db56d309173b4f7d7c958013c97ad3287cdc829f04cc2450fb1e2f2be209b0672d3f7e7901835443a929224281b1ebde519944473ca5f72d5e602a731a551b7eaf4f2c4bc68c9fb512842dec6fb2f6ca9d8a672397b1a88f170b47ad1472a7d8a49e65f286f3e7651691d210bf88fffff014719e9c2c6868a4ae500c691f334f735de8151f4730d3417d30f57dcb8af3f3d91cde8e9e9b9c52490a489453cde886b68f30dadc812fc88c4f8d65c883a4daa49c48fd8a8561586bb0b101dc0fdf6d9f369ba74e83e043ca44052488d905d10d4fcdc73906875bad06c4e640f0dbaf39bebee64f05146c80d0b442dfba1785218957e03d57f36d98ef1466f69aab506ae4e1c2326a02f45a3ac3a23bc40304daa15619eba685bc4a10db798bc1d8d6ec328d06726e648ecf2a3ed35a7a79e8fdf8049b2798ffe7e0162549814c498483c943f49e603fa7e527f231652d55e37e83c49c75677cca772d42ea256f186fbb94aa0332c6355649876f6fc6a55556bcec69e94ff048781e5312723ee5f0cb3dd5f6ff28524d95db5355be9a04607ae2de5baba15a73f10225faef60b1b7cf02f853f9fb3e9295967191627574d5451855e2287f7af50392e500751202d36ecc71e61f7c5a68af419d724707e8f04f21b225c5c881e13ddf392c9480e3dc0ecd0dfad68dbb0c88febe1956569d703d2aaf423a1d0b11e8ad3aeeb19819e76086f5b3dab75dce7effcdfe"),
            0x1f7fffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x003f15eade9586446b8dae5ff3eca036c8829dd695aa891738f0972ab3c2c92c"));
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
                {0, uint256S("003f15eade9586446b8dae5ff3eca036c8829dd695aa891738f0972ab3c2c92c")},
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
