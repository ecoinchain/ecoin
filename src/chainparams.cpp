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
        const size_t N = 200, K = 9;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        // 网络部署时间跟genesis设置的时间不能超过1天
        genesis = CreateGenesisBlock(
            1523876400,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000000a"),
            ParseHex("0017bb52a90c0d94fd9344bc5151a0e1bc2ade6bff1ef44d1bfb28136d8f12645cdbc60ca1eacf18c9de2ed52200c21ee65d0de537cd2be1534379021ed533412516c991e5b3413c333460e9635e7e2e04f44ddd19dacb588a46ea130dad139f07db351e733a7a716333de63a474de18a954a355e9c6f7f58a9ce11b8fd32b6e237832114eecc924b70f7fce431a8b29d54c70369fe1e9e398ef715b7ff75bd24d8196318a77caa8007cfb3e53ddab431c4ee24de17ab47493befe12ef072e74f4e066eb45386be2c9f07b4315dd60f8cdef097ba928d55122299db9819b7a453eab5c053c33580be498893b546b57d536c0e983ab37a0f2fae9200c184cc7756be0da27e26844ab46dbc2dd4d5dd3b394628fa7b2f06c7ea37e1f6b2a816cdc4b6764dd4de01eeb272acbea45617164d23616a3714116ab4a7d2d2efb939df1979b43d228d4857ef2222d8d4194bf0b00835e9fde87285b4938203c9c72add4f5d0b927eb619a16641728538bf4d1b66e87bd0146144bb309901896ac1691895eb2bea1c7eedd5058b62e0b3eb97824658dc000cea33675e4941bb1f5e68d2ec018ab1d023f9e9aa5b67e31f580231762bd59e6b955dc633805159553b8d45b7962159098495855183fe21e22630cda59b4f9cdc4994b8cf3d868be4e1a48717c2e3c1c7c8688712188d790ac061304f8f2d2d00719e68f079f2f0b7947cea0427a0909ff63e506d6725da48115fd9ec3c826f83b39ebd2c5dade216568863877c712eda89b7bc57ec5ff5454b63165d813258ddb8c581ae1bec90e18a07cd40c157fd5d502b5cdb15dee560b93195e119869c1ba8871477b738058a7f4b374553f681a5a0993055b1f6c95b8e64d53e35b03bd708f0d76207c140c6af48e2aa4e894ea7b79f99096d229207b9e86184b788974d0325dddeea351e490b7328d00ef42073607118d20af113ab9dac34cf0da71138d20c0074700d33b1ffa0b69303ef92fee7d6e1f3bf80fbeb937539ed1afee9381d16533ad4134a913ebe1169724ed8d8b306e75e936a970472cc27c03f57e0905846d4856d06472a75cc54a49660e49d471d2b2c63263249785e1db4d5aae76daf265651ea8ffba35cc20a3a30efe4c699add37e309ec66ae70fc3ceeaab140a8ed3dab6db307e11fa7a93d4c199a8b2a9875ec08caa0df37ccadacf856458180ceb8071b1a1983c910d269dfced9931f1524f74947f39f12ffc8ba7150435ddfa5a41b8b3bc64e952de2dcbc3dcf64d60d60632285728feb185f705476630ee70742da593d92c90d87bb52d04608412778d32bc5294491ad6cbc63bc3297b76ce84d391b78fd14b9f640541d8c43fc57231721e0e9118cb6eb46c4b6efb26a76a5dee29cd8ef33cc737a9e0d0d73177473ab2855b2c5dda47447030176f8658f0df0ed077660c58cb2f59e3d3971fbc43ae38cabcdd1796f9c26b585cab831520c47b7795301a56de28c20204bf147dc4de765263f88473e374a04ee298ce69c1e537d86f156f42565a4b115ff5d6403191f50635cab6d1ca8537c2c6a6fc99b0113c51a099d09a880c571b9302673a1c2621ed722619a0b7c2f2369e5a36a22cd6f62a4be8d4fe84273c35c07e72f6152dc0e1941f3f34333d4f5cff5c37c3bdc95d2017d52679885a4b4eec451d04c24f13504ef3b4f3402d036d3b5a4bcf5adc0f0c12c06b20a2c453cb02202a4d44198d492eb3a350634c057cab6012f3384f939a573119872b86b983e460c4d78048afae07b05ef080ff6f4aa87d68867ee416f2172875d94561905e409c2e316f459834b10df35c506b046f9a8b3102d75106b5a8e83c44c771208b604bf5b61424c08bf61972ae89da0fb6c3dbf84c8249afa67779546db72625e"),
            0x1f7fffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
         printf("main consensus.hashGenesisBlock = %s\n", consensus.hashGenesisBlock.ToString().c_str());
         printf("main genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
        assert(consensus.hashGenesisBlock == uint256S("0xd3f221be133385d5241004b27847b2cd07434154435703bb983b9c91f0298c37"));
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

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,33); // E为地址的第一个字母
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,15); // 7为地址的第一个字母
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x14, 0x03, 0x12, 0x42};
        base58Prefixes[EXT_SECRET_KEY] = {0x43, 0x5d, 0x8c, 0xb2};

        bech32_hrp = "ec";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        // todo 区块链的检查点，等到链成长到一定程度再设置
        checkpointData = {
            {
                {0, uint256S("031fcb90377f5fdae730e3689f36d032cbcc036c27a935bf56a08ed958da8b2e")},
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
        consensus.nSubsidyHalvingInterval = 2000000;
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
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
        const size_t N = 200, K = 9;  // Same as mainchain.
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        // 网络部署时间跟genesis设置的时间不能超过1天
        genesis = CreateGenesisBlock(
            1523876400,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000010"),
            ParseHex("004a518bb3de797db13f33f100bd9d112b5b9b6aca17f1fca041fbe47bf04c68d4accecfaa83e1b5699c08a91dff368e66e11e27994ce8eecdd2b8e27b395132e5cac61d547b9fbecbe4e9024040459801b0c69a07eb20691f30df5794aca6a0187582aba487ff5f8c4c3e16cd6fb9e88ddb9205f257d8311e713bf468120b5bb456839f6aa34408183c4ec7d3c7603fdd1b8422092fdea923fabf488f9220f0fb932d4707d56a2d06d42a7e200434294d6451b14d65bb7d7e03d9a79d2b42ee79496956e5a1de08b38649dc6ecf04f707a723c42f56546c895fbea2d71e27c31bd67db1bc7e8e3d166794d71dc1a5f3f3f53c80ec1a6a38dd1247d115408b8ceb57fd42c0ad353925c822f7308abf13941d64053a32d3fc83a3e557084ddede71ed7151a1a41d14720622a2df393e70036b40e54f26197a9eaedb1f6ac29841c861fc455332664fe8dc8514ba2e702101b5a98805e7e43771e1d07a0855fea8ee15b5b1fe22c7576eae4918692779a27afefa7332106b138a0909cc189ae7aac371f6258650b87eba499f221c611c0edc127365a04d0b5bfce61156c8761763099e77560f0ad4cd28d4ff355d6fd3b52fdc1cc99f5ab074430feeb7e631d22974c116a39a4bd4124d915573b857207a6cd73773d2fda2e132ea58cf673357559b91004a06ae62fca407e56af6d52e6c3ced2daec0ba1e7108bbbdbfe39ee575013e41618fb834f529b73f4c7a1da7a142dd5e2e21eaa3742d736e80a676ca756f2309c4d26610cb2933b8b2a4444e2fd15ebb809b74db10021474ff625d07ddda7579313d04e57b66779b661c3d69d0105fbeafefc5c31adcaa915a89f2dc26dd3e715c5473d43f8d7e58142f78451d2e217bfe3d022b1e23cda9967d4b14168388c4d9327db259709c283c289cd674f000a7c8c4b675d74303ae842cd4698400e333702c87ec8a4d84461d1bd61abe766939b6cc33cd1c86f594127b0ab0d784decb95524294bdb37c13a04c24435176f100d595681be034428f9b3b3d121a67c972779c04c9784223ce2d39a4f9213dcf851201e8c2f596b228b7e29066631e4d752db3307a9cfa0ddc74fdc68dcebd91cd24a558d86cc13d89bec69e0453c9bb602e99c3e949f5ad7b645a4dd6e196734352a5078edb1cc90b574c55c26c500e46b8033ddc8d02b68dfffa574d73e240719ad6701128f5f8c91a7007e128b65e163501d5b51329d3abbc1a64f2374e6705ddc486cd6cead56997e1b91cfecc78d29cd8df360b8c20cd656efd25c044112a509c0828ba7acc107003e4e0c596480a716b5e75b46f52e9de75971a5cd8199541c1a9da16f544b3a45ffd24b8a1312f3171f30544cd3c544d4484aff853acb9f59307386d7a874f2347261a538a98a5cd275731404895678df79e5f71025dfb4bc00bde03edade7c3f5ce3baa3e6d1251b414cbface83986671554a61c399bbb531410a7441e819b76105bb58141d5f4fe1ec2bc495d488be7951072c581385092995d7be4b262606dc87d1bad83999d420da8d638152a8a92455e398fd64fa1e6c8b9377ef3a1353deb06c4cbb762ba3c238ccb74dc2655a646f2837e7042dca7cf0c61c955aad7c85557368cda0c12d64e67538d20543549c54271461aae54405fb1fcb04ff9aaacf45f5203bb749cc96d2318fc9dbbeb9000d0b11889a9b1905b577f0ee81fa7525590aeefc321f074aaad00d0e8b9949b1f58ce0ad9d78839b70203abf9fc4bc6885495207667d0de97cc619e1df17bf0d125f50e6e06dffda98618955fcda50e5c6b306131287718b6508063ffc85c439cef6ef52f853f9adb0193101ef329c297f3c2cb6d9077f54e9c93cf5558d30f19b76becf57cd0f94e65c4e695f1aac41bdc08c"),
            0x2007ffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x003a34ca181576904c06389fbaf42a8c3ada59e44426afd7a00772b50f579b63"));
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


        // todo 区块链的检查点，等到链成长到一定程度再设置
        checkpointData = {
            {
                {0, uint256S("003a34ca181576904c06389fbaf42a8c3ada59e44426afd7a00772b50f579b63")},
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
