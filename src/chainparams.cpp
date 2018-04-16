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
        consensus.nSubsidyHalvingInterval = 200000;
        consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2 * 24 * 60 * 60; // two days
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 274; // 95% of nMinerConfirmationWindow
        consensus.nMinerConfirmationWindow = 288; // nPowTargetTimespan / nPowTargetSpacing
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
            1523773297,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000000a"),
            ParseHex("0017bb52a90c0d94fd9344bc5151a0e1bc2ade6bff1ef44d1bfb28136d8f12645cdbc60ca1eacf18c9de2ed52200c21ee65d0de537cd2be1534379021ed533412516c991e5b3413c333460e9635e7e2e04f44ddd19dacb588a46ea130dad139f07db351e733a7a716333de63a474de18a954a355e9c6f7f58a9ce11b8fd32b6e237832114eecc924b70f7fce431a8b29d54c70369fe1e9e398ef715b7ff75bd24d8196318a77caa8007cfb3e53ddab431c4ee24de17ab47493befe12ef072e74f4e066eb45386be2c9f07b4315dd60f8cdef097ba928d55122299db9819b7a453eab5c053c33580be498893b546b57d536c0e983ab37a0f2fae9200c184cc7756be0da27e26844ab46dbc2dd4d5dd3b394628fa7b2f06c7ea37e1f6b2a816cdc4b6764dd4de01eeb272acbea45617164d23616a3714116ab4a7d2d2efb939df1979b43d228d4857ef2222d8d4194bf0b00835e9fde87285b4938203c9c72add4f5d0b927eb619a16641728538bf4d1b66e87bd0146144bb309901896ac1691895eb2bea1c7eedd5058b62e0b3eb97824658dc000cea33675e4941bb1f5e68d2ec018ab1d023f9e9aa5b67e31f580231762bd59e6b955dc633805159553b8d45b7962159098495855183fe21e22630cda59b4f9cdc4994b8cf3d868be4e1a48717c2e3c1c7c8688712188d790ac061304f8f2d2d00719e68f079f2f0b7947cea0427a0909ff63e506d6725da48115fd9ec3c826f83b39ebd2c5dade216568863877c712eda89b7bc57ec5ff5454b63165d813258ddb8c581ae1bec90e18a07cd40c157fd5d502b5cdb15dee560b93195e119869c1ba8871477b738058a7f4b374553f681a5a0993055b1f6c95b8e64d53e35b03bd708f0d76207c140c6af48e2aa4e894ea7b79f99096d229207b9e86184b788974d0325dddeea351e490b7328d00ef42073607118d20af113ab9dac34cf0da71138d20c0074700d33b1ffa0b69303ef92fee7d6e1f3bf80fbeb937539ed1afee9381d16533ad4134a913ebe1169724ed8d8b306e75e936a970472cc27c03f57e0905846d4856d06472a75cc54a49660e49d471d2b2c63263249785e1db4d5aae76daf265651ea8ffba35cc20a3a30efe4c699add37e309ec66ae70fc3ceeaab140a8ed3dab6db307e11fa7a93d4c199a8b2a9875ec08caa0df37ccadacf856458180ceb8071b1a1983c910d269dfced9931f1524f74947f39f12ffc8ba7150435ddfa5a41b8b3bc64e952de2dcbc3dcf64d60d60632285728feb185f705476630ee70742da593d92c90d87bb52d04608412778d32bc5294491ad6cbc63bc3297b76ce84d391b78fd14b9f640541d8c43fc57231721e0e9118cb6eb46c4b6efb26a76a5dee29cd8ef33cc737a9e0d0d73177473ab2855b2c5dda47447030176f8658f0df0ed077660c58cb2f59e3d3971fbc43ae38cabcdd1796f9c26b585cab831520c47b7795301a56de28c20204bf147dc4de765263f88473e374a04ee298ce69c1e537d86f156f42565a4b115ff5d6403191f50635cab6d1ca8537c2c6a6fc99b0113c51a099d09a880c571b9302673a1c2621ed722619a0b7c2f2369e5a36a22cd6f62a4be8d4fe84273c35c07e72f6152dc0e1941f3f34333d4f5cff5c37c3bdc95d2017d52679885a4b4eec451d04c24f13504ef3b4f3402d036d3b5a4bcf5adc0f0c12c06b20a2c453cb02202a4d44198d492eb3a350634c057cab6012f3384f939a573119872b86b983e460c4d78048afae07b05ef080ff6f4aa87d68867ee416f2172875d94561905e409c2e316f459834b10df35c506b046f9a8b3102d75106b5a8e83c44c771208b604bf5b61424c08bf61972ae89da0fb6c3dbf84c8249afa67779546db72625e"),
            0x1f07ffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        // printf("main consensus.hashGenesisBlock = %s\n", consensus.hashGenesisBlock.ToString().c_str());
        // printf("main genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
        assert(consensus.hashGenesisBlock == uint256S("0x0006f2d547a5e2a1fd879e8be7be41be6336c9867ba2ff6f8d0ebf1c2787da69"));
        assert(genesis.hashMerkleRoot == uint256S("0x83aa4396221b56f76352e72a20be936493b03c4c15f368e266613ee645f9256f"));

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
                {0, uint256S("0006f2d547a5e2a1fd879e8be7be41be6336c9867ba2ff6f8d0ebf1c2787da69")},
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
        consensus.nSubsidyHalvingInterval = 200000;
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2 * 24 * 60 * 60; // two days
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 216; // 75% for testchains
        consensus.nMinerConfirmationWindow = 288; // nPowTargetTimespan / nPowTargetSpacing
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
            1523498400,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000000d"),
            ParseHex("004e8dfa09ca8fbf2fee30a9ba552bd438dd6eb20b01c186a994c7fd2161ae203e2063b8d53efa3afde6033f202f8b21c8afda6a33732de8ef75a82b0fe80022714a6b1e657c0b943024776dab1495f5ec1c4fd0086898d1fda17ab370b174eb8f48c8f2bc9078dd583a052ff6a97bb335ec9303a3e3f6da4518f4ac4ad81ae24acfd0d8756b849f92c01361a45add757d113347c287ab3515c8abcdef753b0371045e3ce875ca0c009e4f7da6d99739f008d73122f41c7a8dfad8027f07861a848c4b9db6c2ad33e04ed7fafd20add6017216bdd754ba072948697cc51334487b9164f7acd3ee46b722e1d39eaeb92a429522acd05a4ddcd03baf080f6fa3664cd3ca79a953142aa4760ef978bceec7b15d01c5a00d76907fc23b85d19b45be5259b5b5dfab18e7c9f9914cdc1b62a5d4916de72dd5b1ffdfbf4a5692e33a9d1bfa6513bb36e3bce76245e51fb11a8c04ae67f9e1c7f590bd88d0c8d7957378fbaca90a531649f3230507df97497b379d08e52ece6201f61aba0caacdf3e609a711b58b311460f16e385194a8ade5114e441a44a651a7a9f621255edba58d9719f0167326353bb3828f1f8fdf62ba611b6e33abd6421ed90932e65d0a51a8f3ff8d3543a42ccd5212543af2c86828a335690dda26736a32e614d0ec90670ec21d0d15423c7d1f789a48a6fb5576a17f3a6b79ec415b29850cc0c3ed78259cdb9905585426f738faa23fd5dfd6237182e2b86316492845a3dd2d4ab08b50739aec021e2b5189654e5ebba78ab3ad2dc268fcf768350b6124741f5eeada75412457e6adbfb5e86b4bde3f689b0ce9766f112b7f51827081d9b6a60ef9058a93cf83136f44c7558b6e72c7fe9278d8c26788a3d9b85a9825a9b2821da03f3f787319a0d67c75f27d84fc046034ddfeee2d732fb3f25767c945676e6f64d71b8a4701196def428373937a16782159567ff687ab37db2f1532454016e8fea9e2a943a1f6a0c5fd9b6c9aa36d2343c28bb55a54e1ec36b25dfbfd3114fc557adfc0272e2e46aa8ee76e8790384fa9655b3a3d9f9f573f0d307d104103adc22bbcb42d7450d8c2f5f95d0f050e08890648e281135d67f21306a74f68ec5cb88b8c15791e9f2948c880ee68f3f98d7f01c12e08bb7e4a2708edde338b560eef7892861379bc10c9197a3a2b0f8cd97f9e959257db829298fba7202a88ffb55c1312a0959f3e614d73caad71a09c2b8dded4c01e02ad3a45372ff3da26e9eae33951f55b03aa8407165435479bc71ec1a09a891c2fb697933d369ddf762f086c175db4f636a3afbbe9f4d25a71c14c8992a03b7d213a603695cfdc9f054e86659351c1fc899d2294433a22af6411c94f7cf4d4fb95c332cbdd4618b5f5dbb42e35527da50f964abc366465b5e6f70e9b4eb697b705a0dec234ee514bbcddf1aea1542d128801948740485032629b75ce97c074f566f5545f25ff48d5c56714e237883f474fe9da5003ab1d672305239778bc3d155d717ad5d21a72a2fbb29b7eb8059ccdb831103b0c4137acac463e45604b96a7e9e6c9a5fa107f30df20cf14c5e61a96a5a2e3176a515b41d25509fd0ac71f1f522789aec9f189b2252af9337c462bfef56c6142991afd1735dabdf23aaa99cb5b2cc3a589dde0000a7982c5ea53597fb44d0298a4ac9959a9731b8d1b0ecc90a7dc4bcf42a705b5bf77405e95d4e35f82410cd6bd3e36e57b7dc39c62e87b785729d406168a331ea6ca74e88fc5f70629146ed9645b3f2e5f7b3276105a82cec00a790ec6ea134574a5f4ed53dc6bec4a39cf0407e1502b16d2975480ed65be9593419c487f3fcc937436941ff7b0f554faa6f9b241e66416578148bcf427d29385de9cf12ab9c77db6b714b4ba44c6"),
            0x2007ffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        // printf("test consensus.hashGenesisBlock = %s\n", consensus.hashGenesisBlock.ToString().c_str());
        // printf("test genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
        assert(consensus.hashGenesisBlock == uint256S("0x01414ba30e700a0534b98aab8f2340339ce3a3f20649405e2fa7e2135315072d"));
        assert(genesis.hashMerkleRoot == uint256S("0x83aa4396221b56f76352e72a20be936493b03c4c15f368e266613ee645f9256f"));

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
                {0, uint256S("01414ba30e700a0534b98aab8f2340339ce3a3f20649405e2fa7e2135315072d")},
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
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000005"),
            ParseHex("00ea58f6f75d6e192723f9172ea79eee15320b08c98558ae5e47e0325a531b65f7561d2b"),
            0x207fffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x4130a5b7d7b9d532b5e006b7e1d557cbfd311fcd819289477135efba4368efb1"));
        assert(genesis.hashMerkleRoot == uint256S("0x83aa4396221b56f76352e72a20be936493b03c4c15f368e266613ee645f9256f"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("4130a5b7d7b9d532b5e006b7e1d557cbfd311fcd819289477135efba4368efb1")},
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
