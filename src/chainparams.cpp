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
#define equihash_parameters_acceptable(N, K) \
    ((CBlockHeader::HEADER_SIZE + equihash_solution_size(N, K))*MAX_HEADERS_RESULTS < \
     MAX_PROTOCOL_MESSAGE_LENGTH-1000)

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
static CBlock CreateGenesisBlock(uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
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
        consensus.BIP16Height = 173805; // 00000000000000ce80a7e057163a4db1d5ad7b20fb6f598c9597b9665c8fb0d4 - April 1, 2012
        consensus.BIP34Height = 227931;
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.BIP65Height = 388381; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.BIP66Height = 363725; // 00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
        // todo 临时使用test的参数
        // consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1479168000; // November 15th, 2016.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1510704000; // November 15th, 2017.

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000f91c579d57cad4bc5278cc");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000005214481d2d96f898e3d5416e43359c145944a909d242e0"); //506067

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xf9;
        pchMessageStart[1] = 0xbe;
        pchMessageStart[2] = 0xb4;
        pchMessageStart[3] = 0xd9;
        nDefaultPort = 8333;
        nPruneAfterHeight = 100000;
        const size_t N = 200, K = 9;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        // genesis = CreateGenesisBlock(
        //     1231006505,
        //     uint256S("0x0"),
        //     ParseHex(""),
        //     0x1f07ffff, 1, 50 * COIN);
        // todo 临时使用test的参数，main的参数计算一个合适的nonce耗时太长了
        genesis = CreateGenesisBlock(
            1296688602,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000000e"),
            ParseHex("00b0c76a03930fad346cf2e6254e1aae582cd93ba2644b853a6a20e549d62196791fd39f1dcc7ad063470224ec15fa0deddf760591193d9adf51665b545c481d570baecd6befb5c5891348dab078155125f0918f05806bf6786de2957cac8b1fd4f0ae46f3151ec1770f1f014b1e33194bf2b0c3cfff4c80af5c56fcc8630e63bed5e1ddab25cc388aa203f5009aeec357f9df2484458c43ce0999a17bd58db26e535179dfcceff0019a113d4562cb89fe28801d76b5cc6cb1ba73be780bfc94cd65f9d6a7e3b2344986a8975982b1904c6a0591e700ea8c8c5fd8f881294568e360bb216e09474f566303553094e39d6196c079c7fcea8c47b479600b6db260e25ccd19d754e46e1635834f382c9ec2ba1db07dcf004bd8cf854f41feda6d9c0239789821f50dcb38b3c3154b3daaa512d1afe5dc40c36758bec5261cfb96815474a4f1cbc6675962cb17083db91aee01b0fec733ca1d7f720da06876bbbd85b4ba9f7c42111b3ad5ac93d21361df51d95dd26c2f74c09f5fe90562fe03b7624235f60b50f122b152c92c435bc7c433dd61e2226112a5963997a3f3e6cf3622345d9441046ed419968fee65d61530b94d1286ecffccbde23714597e7c3f0bb35f21ec9250263cdd6364017c021c07ec7f1b084b898f1585d6a497f7e66a00093fd8fc3ceec685ac6b2127af6b948898de0a176449dce11e0223c87dd909a5a93ff726e750d6539a74dc97b263137fa4085d2095c5d211a27574ea4e25987953bfcd051e72f5a5a31e0bffb8879355dd63b6923a18718a295204064666629ba03e3804444ccdda4782b402e905654ef44ba797c9650a958c05af3a0368dadef9e20aac89ae729d5459a091e54b9ddea126678ef7904a0d242b2752dfaecfc0e1b2a3157892e0df62f60d651ef46501a8ce5e26fc2ba213f1d3898517abaa695a00fb9ea19669230b834ca078d29647c2538fd84e780bbad567b4c87f25922715c6eeda4c533279dac0a21fb9e26a78a3e959a078a20751efa8f54565b5d2a34caa428c2ee17b19d083a63626dea7afcc2abf2c091475b46a46237093c67f12353679063d41031f81038c94df574ef21465e992a8f6586fd602537819850b1fd1bb9e69c8116afd01c514fbe4fd1d9db7dad1525cd51c55829d65599c8ab7160bd0b6b64670754160014362a7a5f6c1e5f1f2f0efcb4c4370516a5abcee0caea183732008d99c92a3047635350122766fb317059f13241b01e24a834f414ec39ac948afd072bd2706698d1a9dea1b59a0c22411b55ef5aa1fa316d7c5069303ccefba8b31fa8ec4517aa77d298c60759c5f07ddf30d961c07a0e0b4824364dac700b6ee27817824bc8e78640dc750e27ff81297f711274615fdc86643bf873ee216c1fd640d790d5e4ba88f02051d92fb0172bfad930e0eeedf94f0648978674d0cf73d8aa6275c91e3549a0f170c6f7326dacaf1ae386c7303fe1d6ce15dcaadb9a9de13e1e5fa33b72e5d3114ea1934aa46c4cc25c77b9b67e47b09ab03e6b36a388edc05c69e5d63543d8b0d4ae23311e136096c3fec0690069f176033442117eed3d182abb89c7e6a3f95b01015511fa2eb5a22bfcf8494b77abfa202abe9d7faaf31a644bc4bdca54359b1e4db236c6e0f2df2ddd08202eeede6736dbef782f27438de2845ae024fd79ba51c421eab0b89ac9c814cc2bea0738b356d0510200608ea255cfc8d633d31ba8708d8ccd0d248e4fe18ec406eaaa0e49125daa8fa274c3f658a3ec3c136be350fc63e8f57882d934b38d1098399953a441ef380eb4d33a283621b17d31436166812d63b466c799ef6067c77b604e7669e51d271498b28ed3a8311de1c46ba97321d076dea1593525e8ba7fd60fcdb071fff837d"),
            0x2007ffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        printf("consensus.hashGenesisBlock = %s\n", consensus.hashGenesisBlock.ToString().c_str());
        assert(consensus.hashGenesisBlock == uint256S("0x07fee3c7c3999c062751436d29af5b24d4989eecda41acfbea3d628edd9570ae"));
        assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as a oneshot if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
        vSeeds.emplace_back("seed.bitcoin.sipa.be"); // Pieter Wuille, only supports x1, x5, x9, and xd
        vSeeds.emplace_back("dnsseed.bluematt.me"); // Matt Corallo, only supports x9
        vSeeds.emplace_back("dnsseed.bitcoin.dashjr.org"); // Luke Dashjr
        vSeeds.emplace_back("seed.bitcoinstats.com"); // Christian Decker, supports x1 - xf
        vSeeds.emplace_back("seed.bitcoin.jonasschnelli.ch"); // Jonas Schnelli, only supports x1, x5, x9, and xd
        vSeeds.emplace_back("seed.btc.petertodd.org"); // Peter Todd, only supports x1, x5, x9, and xd

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "bc";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                { 11111, uint256S("0x0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d")},
                { 33333, uint256S("0x000000002dd5588a74784eaa7ab0507a18ad16a236e7b1ce69f00d7ddfb5d0a6")},
                { 74000, uint256S("0x0000000000573993a3c9e41ce34471c079dcf5f52a0e824a81e7f953b8661a20")},
                {105000, uint256S("0x00000000000291ce28027faea320c8d2b054b2e0fe44a773f3eefb151d6bdc97")},
                {134444, uint256S("0x00000000000005b12ffd4cd315cd34ffd4a594f430ac814c91184a0d42d2b0fe")},
                {168000, uint256S("0x000000000000099e61ea72015e79632f216fe6cb33d7899acb35b75c8303b763")},
                {193000, uint256S("0x000000000000059f452a5f7340de6682a977387c17010ff6e6c3bd83ca8b1317")},
                {210000, uint256S("0x000000000000048b95347e83192f69cf0366076336c639f9b7228e9ba171342e")},
                {216116, uint256S("0x00000000000001b4f4b433e81ee46494af945cf96014816a4e2370f11b23df4e")},
                {225430, uint256S("0x00000000000001c108384350f74090433e7fcf79a606b8e797f065b130575932")},
                {250000, uint256S("0x000000000000003887df1f29024b06fc2200b55f8af8f35453d7be294df2d214")},
                {279000, uint256S("0x0000000000000001ae8c72a0b0c301f67e3afca10e819efa9041e458e9bd7e40")},
                {295000, uint256S("0x00000000000000004d9b4ef50f0f9d686fd69db2e03af35a100370c64632a983")},
            }
        };

        chainTxData = ChainTxData{
            // Data as of block 0000000000000000002d6cca6761c99b3c2e936f9a0e304b7c7651a993f461de (height 506081).
            1516903077, // * UNIX timestamp of last known number of transactions
            295363220,  // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            3.5         // * estimated number of transactions per second after that timestamp
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
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP16Height = 514; // 00000000040b4e986385315e14bee30ad876d8b47f748025b26683116d21aa65
        consensus.BIP34Height = 21111;
        consensus.BIP34Hash = uint256S("0x0000000023b3a96d3484e5abb3755c413e7d41500f8e2a5c3f0dd01299cd8ef8");
        consensus.BIP65Height = 581885; // 00000000007f6655f22f98e72ed80d8b06dc761d5da09df0fa1dc4be4f861eb6
        consensus.BIP66Height = 330776; // 000000002104c8c45e99a8853285a3b592602a3ccde2b832481da85e9e4ba182
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800; // May 1st 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800; // May 1st 2017

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000002830dab7f76dbb7d63");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000002e9e7b00e1f6dc5123a04aad68dd0f0968d8c7aa45f6640795c37b1"); //1135275

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x07;
        nDefaultPort = 18333;
        nPruneAfterHeight = 1000;
        const size_t N = 200, K = 9;  // Same as mainchain.
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        genesis = CreateGenesisBlock(
            1296688602,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000000e"),
            ParseHex("00b0c76a03930fad346cf2e6254e1aae582cd93ba2644b853a6a20e549d62196791fd39f1dcc7ad063470224ec15fa0deddf760591193d9adf51665b545c481d570baecd6befb5c5891348dab078155125f0918f05806bf6786de2957cac8b1fd4f0ae46f3151ec1770f1f014b1e33194bf2b0c3cfff4c80af5c56fcc8630e63bed5e1ddab25cc388aa203f5009aeec357f9df2484458c43ce0999a17bd58db26e535179dfcceff0019a113d4562cb89fe28801d76b5cc6cb1ba73be780bfc94cd65f9d6a7e3b2344986a8975982b1904c6a0591e700ea8c8c5fd8f881294568e360bb216e09474f566303553094e39d6196c079c7fcea8c47b479600b6db260e25ccd19d754e46e1635834f382c9ec2ba1db07dcf004bd8cf854f41feda6d9c0239789821f50dcb38b3c3154b3daaa512d1afe5dc40c36758bec5261cfb96815474a4f1cbc6675962cb17083db91aee01b0fec733ca1d7f720da06876bbbd85b4ba9f7c42111b3ad5ac93d21361df51d95dd26c2f74c09f5fe90562fe03b7624235f60b50f122b152c92c435bc7c433dd61e2226112a5963997a3f3e6cf3622345d9441046ed419968fee65d61530b94d1286ecffccbde23714597e7c3f0bb35f21ec9250263cdd6364017c021c07ec7f1b084b898f1585d6a497f7e66a00093fd8fc3ceec685ac6b2127af6b948898de0a176449dce11e0223c87dd909a5a93ff726e750d6539a74dc97b263137fa4085d2095c5d211a27574ea4e25987953bfcd051e72f5a5a31e0bffb8879355dd63b6923a18718a295204064666629ba03e3804444ccdda4782b402e905654ef44ba797c9650a958c05af3a0368dadef9e20aac89ae729d5459a091e54b9ddea126678ef7904a0d242b2752dfaecfc0e1b2a3157892e0df62f60d651ef46501a8ce5e26fc2ba213f1d3898517abaa695a00fb9ea19669230b834ca078d29647c2538fd84e780bbad567b4c87f25922715c6eeda4c533279dac0a21fb9e26a78a3e959a078a20751efa8f54565b5d2a34caa428c2ee17b19d083a63626dea7afcc2abf2c091475b46a46237093c67f12353679063d41031f81038c94df574ef21465e992a8f6586fd602537819850b1fd1bb9e69c8116afd01c514fbe4fd1d9db7dad1525cd51c55829d65599c8ab7160bd0b6b64670754160014362a7a5f6c1e5f1f2f0efcb4c4370516a5abcee0caea183732008d99c92a3047635350122766fb317059f13241b01e24a834f414ec39ac948afd072bd2706698d1a9dea1b59a0c22411b55ef5aa1fa316d7c5069303ccefba8b31fa8ec4517aa77d298c60759c5f07ddf30d961c07a0e0b4824364dac700b6ee27817824bc8e78640dc750e27ff81297f711274615fdc86643bf873ee216c1fd640d790d5e4ba88f02051d92fb0172bfad930e0eeedf94f0648978674d0cf73d8aa6275c91e3549a0f170c6f7326dacaf1ae386c7303fe1d6ce15dcaadb9a9de13e1e5fa33b72e5d3114ea1934aa46c4cc25c77b9b67e47b09ab03e6b36a388edc05c69e5d63543d8b0d4ae23311e136096c3fec0690069f176033442117eed3d182abb89c7e6a3f95b01015511fa2eb5a22bfcf8494b77abfa202abe9d7faaf31a644bc4bdca54359b1e4db236c6e0f2df2ddd08202eeede6736dbef782f27438de2845ae024fd79ba51c421eab0b89ac9c814cc2bea0738b356d0510200608ea255cfc8d633d31ba8708d8ccd0d248e4fe18ec406eaaa0e49125daa8fa274c3f658a3ec3c136be350fc63e8f57882d934b38d1098399953a441ef380eb4d33a283621b17d31436166812d63b466c799ef6067c77b604e7669e51d271498b28ed3a8311de1c46ba97321d076dea1593525e8ba7fd60fcdb071fff837d"),
            0x2007ffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x07fee3c7c3999c062751436d29af5b24d4989eecda41acfbea3d628edd9570ae"));
        assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.emplace_back("testnet-seed.bitcoin.jonasschnelli.ch");
        vSeeds.emplace_back("seed.tbtc.petertodd.org");
        vSeeds.emplace_back("seed.testnet.bitcoin.sprovoost.nl");
        vSeeds.emplace_back("testnet-seed.bluematt.me"); // Just a static list of stable node(s), only supports x9

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tb";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        checkpointData = {
            {
                {546, uint256S("000000002a936ca763904c3c35fce2f3556c559c0214345d31b1bcebf76acb70")},
            }
        };

        chainTxData = ChainTxData{
            // Data as of block 000000000000033cfa3c975eb83ecf2bb4aaedf68e6d279f6ed2b427c64caff9 (height 1260526)
            1516903490,
            17082348,
            0.09
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
        consensus.BIP16Height = 0; // always enforce P2SH BIP16 on regtest
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
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
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;
        const size_t N = 48, K = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        genesis = CreateGenesisBlock(
            1296688602,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000005"),
            ParseHex("025146de53935119fa0d668f5eb2ea2b49d40ea3da2da2c65ef1a12be70df69d6f8b87fd"),
            0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0d3d094c0925b525fbcd40e1548529adc02272ce440efa653d4608035aac9723"));
        assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("0d3d094c0925b525fbcd40e1548529adc02272ce440efa653d4608035aac9723")},
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

        bech32_hrp = "bcrt";

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
