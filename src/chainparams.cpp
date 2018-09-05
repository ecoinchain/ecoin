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
#include <rpc/util.h>

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
        consensus.nSubsidyHalvingInterval = 200000;
        consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2 * 24 * 60 * 60; // two days
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true; // 避免算力大幅波动带来挖矿困难
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
        // todo 等到链成长到一定的长度再设置.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"); //506067
        consensus.authorizationForkHeight = 21600;
        consensus.authorizationKey = HexToPubKey("a0273c9482b3945e600add7cac313b7c96707d676ba65b7371425ac41b015246");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        // 用可见的消息magic感觉更好点，rcom表示rcoin main net
        pchMessageStart[0] = 0x52; // r
        pchMessageStart[1] = 0x43; // c
        pchMessageStart[2] = 0x4f; // o
        pchMessageStart[3] = 0x4d; // m
        nDefaultPort = 8866;
        nPruneAfterHeight = 100000;
        const size_t N = 200, K = 9;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        // 网络部署时间跟genesis设置的时间不能超过1天.
        genesis = CreateGenesisBlock(
            1524207262,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000000c"),
            ParseHex("0005cef4edc3f604ab3022b971fc0cfe3ce076e98d01b660759606dbddc71451b37a69e71aeb86f7dc941b7e5994f32711d9a477128903d47ff9ba5abf60fe550cb3f13f29ec3d55a677d2065d6f279538def54b00e3ce5fd04a2055a30e82df8b76f2aea27bdc5b4710c428b91759f69b8f84b3b64dd8dd79211fff658a142c1a7ac40759f2b4510617a4d33205da8caf7b8b15b1ea7e432feac3ee9f07b2624f7b2e066bdf7a2a08e17fb00f07e891ee225791464188d623de1330a70f540b163bcc014e7d8251a93c63ec8cc76f46a0760cb8a152068da28c955c148c0a56e58a518b53c59b12df84764c7d3debf17ec359dd6f6d45d62039b9770afd462cac1b8bab5057534d2a26c6b5ebc1bfeb9631a635d3d4616831308233a64720b57124b61e3b4625a5a58c4e4d438ab5d744de2f75f3fa2451544529397789f8c4a6440d6b32c40d85390f95a84335c8a20123c075809fc8b52b2fa16678474c5982abeeb74e2063c216a4cf8ea9eed9f48104748ce560ba5d203e049397a23ca7bac5b955e5610379ee39c0e859d3113010e2a41e374b3dce2e2399ebcff4558ef5f65d9d0d451fe4e9a3aaf3d18a831e0bf8896b60841c92a52d5276ee565985d7958ba5ddf7fda161ec9a1bccf21faf17929f9d3cd9b29272bad0bc72b156db6bac2f30b54681056044ef570773c9c232ee12416e37829202c133e994c6f6d0958d01f391be28f57825b32a1751c906c68f605ef94a01666b5ad32a43320d5a1de1069049437bc96842ffb0b36abcafa04e0e1af284b22377e9fe468fe5f55684731c17e84c29e7337d6edd08e5ca91a75b3da37d61b1c5d8b7688d0b8730e7391ecef17ab1a82b338ddbd55f1d60d1272548ff56461b0822925a8cfade8c76b34ee77be244f255761d0a38405b07dca975e3734985e33b53676d92d7adc51701c39f7e9d17f79db90942b26bbf25f8ae786c31a601ee601a63dfcae3bbb524140bf9966506a37be3fd12f05c78b00a54d0cf72b4f99d463ceb2162fd2a9535bb1e9d3ea7349b4df6b80501c9aac6c18ef97a630214b9550847ca8fc25a316e3d9a8eb1a11277d5640f50831609d8f83d9c518418abe168512bae1eb763218b5ffc5e4951938dccd36ab32ae92196b436a18d5a95b3c9c2de698afa07ba2432dd66069f2dbe544704b04e19af1021e3fb3ec20d13611c90d89f887c7e0b56bd38edde1d4f0bba513b5b71e8a95f10f127b30892406e2842267a117ee4bf9246a611a0127701f21afdbb1376daf32dfd97429e183879f5c752dcd01208f2f99b310543ee96827a22b771dd6fbc39bf78ec11b57ce29dd89edbb7029282ebbfd1617c880f52d31675773f9433b317c1b112f2cbc4be0e04eaf53c041b773a0ad8d30b9bf8a572e5267687aa36aa9adad201f01f25dd2c93cde5e170360d76704ed4485a3dc30b757892543811cfeebb71faa397ead153ae36a2460779e2565f6844efa25bb1d43d575ec958de2c57a33a31874c50193429b8eb24390fc886766ce593c53d14e6dca03af66357d988a91f7dde4feee3fe99d9332cc64eeedbeb3601fcd0d3c3eb7d87cea45d15df1015be7910e30ec42116e80375fe7fdbe1b7a0f98fab17154ef2b9cb74b6d3bab22466d727ff4e23de14e3025d01c3279004a1d017521e052475dcdaca87ad1a2fdfdadbe7d918732cb6f3cdeac6299549bd4b225405d36325f7d98a8bc070a118526033e8d153c9ffd316b036f091300a999504446746c9b7e633de130b7218d9763675d02cdcd4bc5a3dcf5f3f5ecf99b8a78320106a613b8e5c35cde2f331f6e7a566880e575e8e1d5abef3cd5f73cefe5861ddaf59dfc1956eefcda3390b2b997d598c47dda576c145787f1744e09a3587"),
            0x1f07ffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00079b7c9f50b744473ca944667df7e791cdca969469d9af86ecde1ab597737d"));
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

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,60); // R为地址的第一个字母.
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,13); // 6为地址的第一个字母.
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x14, 0x03, 0x12, 0x42};
        base58Prefixes[EXT_SECRET_KEY] = {0x43, 0x5d, 0x8c, 0xb2};

        bech32_hrp = "rc";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        // todo 区块链的检查点，等到链成长到一定程度再设置.
        checkpointData = {
            {
                {0, uint256S("00079b7c9f50b744473ca944667df7e791cdca969469d9af86ecde1ab597737d")},
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
        // todo 后面等链变长之后需要修改.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        // todo 等到链成长到一定的长度再设置.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"); //1135275

        // 用可见的消息magic感觉更好点，rcot表示rcoin test net
        pchMessageStart[0] = 0x52; // r
        pchMessageStart[1] = 0x43; // c
        pchMessageStart[2] = 0x4f; // o
        pchMessageStart[3] = 0x54; // t
        nDefaultPort = 18866;
        nPruneAfterHeight = 1000;
        const size_t N = 200, K = 9;  // Same as mainchain.
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        // 网络部署时间跟genesis设置的时间不能超过1天.
        genesis = CreateGenesisBlock(
            1523498400,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000000d"),
            ParseHex("004e8dfa09ca8fbf2fee30a9ba552bd438dd6eb20b01c186a994c7fd2161ae203e2063b8d53efa3afde6033f202f8b21c8afda6a33732de8ef75a82b0fe80022714a6b1e657c0b943024776dab1495f5ec1c4fd0086898d1fda17ab370b174eb8f48c8f2bc9078dd583a052ff6a97bb335ec9303a3e3f6da4518f4ac4ad81ae24acfd0d8756b849f92c01361a45add757d113347c287ab3515c8abcdef753b0371045e3ce875ca0c009e4f7da6d99739f008d73122f41c7a8dfad8027f07861a848c4b9db6c2ad33e04ed7fafd20add6017216bdd754ba072948697cc51334487b9164f7acd3ee46b722e1d39eaeb92a429522acd05a4ddcd03baf080f6fa3664cd3ca79a953142aa4760ef978bceec7b15d01c5a00d76907fc23b85d19b45be5259b5b5dfab18e7c9f9914cdc1b62a5d4916de72dd5b1ffdfbf4a5692e33a9d1bfa6513bb36e3bce76245e51fb11a8c04ae67f9e1c7f590bd88d0c8d7957378fbaca90a531649f3230507df97497b379d08e52ece6201f61aba0caacdf3e609a711b58b311460f16e385194a8ade5114e441a44a651a7a9f621255edba58d9719f0167326353bb3828f1f8fdf62ba611b6e33abd6421ed90932e65d0a51a8f3ff8d3543a42ccd5212543af2c86828a335690dda26736a32e614d0ec90670ec21d0d15423c7d1f789a48a6fb5576a17f3a6b79ec415b29850cc0c3ed78259cdb9905585426f738faa23fd5dfd6237182e2b86316492845a3dd2d4ab08b50739aec021e2b5189654e5ebba78ab3ad2dc268fcf768350b6124741f5eeada75412457e6adbfb5e86b4bde3f689b0ce9766f112b7f51827081d9b6a60ef9058a93cf83136f44c7558b6e72c7fe9278d8c26788a3d9b85a9825a9b2821da03f3f787319a0d67c75f27d84fc046034ddfeee2d732fb3f25767c945676e6f64d71b8a4701196def428373937a16782159567ff687ab37db2f1532454016e8fea9e2a943a1f6a0c5fd9b6c9aa36d2343c28bb55a54e1ec36b25dfbfd3114fc557adfc0272e2e46aa8ee76e8790384fa9655b3a3d9f9f573f0d307d104103adc22bbcb42d7450d8c2f5f95d0f050e08890648e281135d67f21306a74f68ec5cb88b8c15791e9f2948c880ee68f3f98d7f01c12e08bb7e4a2708edde338b560eef7892861379bc10c9197a3a2b0f8cd97f9e959257db829298fba7202a88ffb55c1312a0959f3e614d73caad71a09c2b8dded4c01e02ad3a45372ff3da26e9eae33951f55b03aa8407165435479bc71ec1a09a891c2fb697933d369ddf762f086c175db4f636a3afbbe9f4d25a71c14c8992a03b7d213a603695cfdc9f054e86659351c1fc899d2294433a22af6411c94f7cf4d4fb95c332cbdd4618b5f5dbb42e35527da50f964abc366465b5e6f70e9b4eb697b705a0dec234ee514bbcddf1aea1542d128801948740485032629b75ce97c074f566f5545f25ff48d5c56714e237883f474fe9da5003ab1d672305239778bc3d155d717ad5d21a72a2fbb29b7eb8059ccdb831103b0c4137acac463e45604b96a7e9e6c9a5fa107f30df20cf14c5e61a96a5a2e3176a515b41d25509fd0ac71f1f522789aec9f189b2252af9337c462bfef56c6142991afd1735dabdf23aaa99cb5b2cc3a589dde0000a7982c5ea53597fb44d0298a4ac9959a9731b8d1b0ecc90a7dc4bcf42a705b5bf77405e95d4e35f82410cd6bd3e36e57b7dc39c62e87b785729d406168a331ea6ca74e88fc5f70629146ed9645b3f2e5f7b3276105a82cec00a790ec6ea134574a5f4ed53dc6bec4a39cf0407e1502b16d2975480ed65be9593419c487f3fcc937436941ff7b0f554faa6f9b241e66416578148bcf427d29385de9cf12ab9c77db6b714b4ba44c6"),
            0x2007ffff, 1, GENESIS_MONEY);
        consensus.hashGenesisBlock = genesis.GetHash();
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

        bech32_hrp = "tr";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        // todo 区块链的检查点，等到链成长到一定程度再设置.
        checkpointData = {
            {
                {0, uint256S("01414ba30e700a0534b98aab8f2340339ce3a3f20649405e2fa7e2135315072d")},
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

        // 用可见的消息magic感觉更好点，rcor表示rcoin reg net
        pchMessageStart[0] = 0x52; // r
        pchMessageStart[1] = 0x43; // c
        pchMessageStart[2] = 0x4f; // o
        pchMessageStart[3] = 0x52; // r
        nDefaultPort = 18666;
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

        bech32_hrp = "rcrt";

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
