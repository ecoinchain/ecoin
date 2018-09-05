#include <chainparams.h>
#include <consensus/merkle.h>
#include <pow.h>
#include <rpc/util.h>
#include <validation.h>
#include <key_io.h>

#include <test/test_bitcoin.h>
#include <boost/test/unit_test.hpp>

class CAuthTestParams : public CChainParams {
public:
    CAuthTestParams() {
    }

    CAuthTestParams(int height) {
        consensus.authorizationForkHeight = height;
    }

    CAuthTestParams(int height, const std::string& hex_in) {
        consensus.authorizationForkHeight = height;
        consensus.authorizationKey = HexToPubKey(hex_in);
    }
};

CBlock * CreateBlock(CScript coinbaseSig) {
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = coinbaseSig;
    txNew.vout[0].nValue = GENESIS_MONEY;
    txNew.vout[0].scriptPubKey = CScript() << OP_DUP << OP_HASH160 << ParseHex("3bf1b57c1b21c5eb32caf9e1f0c6ff795156dd20") << OP_EQUALVERIFY << OP_CHECKSIG;

    CBlock * block = new CBlock();
    block->nTime    = 1524207262;
    block->nBits    = 0x1f07ffff;
    block->nNonce   = uint256S("0x000000000000000000000000000000000000000000000000000000000000000c");
    block->nSolution = ParseHex("0005cef4edc3f604ab3022b971fc0cfe3ce076e98d01b660759606dbddc71451b37a69e71aeb86f7dc941b7e5994f32711d9a477128903d47ff9ba5abf60fe550cb3f13f29ec3d55a677d2065d6f279538def54b00e3ce5fd04a2055a30e82df8b76f2aea27bdc5b4710c428b91759f69b8f84b3b64dd8dd79211fff658a142c1a7ac40759f2b4510617a4d33205da8caf7b8b15b1ea7e432feac3ee9f07b2624f7b2e066bdf7a2a08e17fb00f07e891ee225791464188d623de1330a70f540b163bcc014e7d8251a93c63ec8cc76f46a0760cb8a152068da28c955c148c0a56e58a518b53c59b12df84764c7d3debf17ec359dd6f6d45d62039b9770afd462cac1b8bab5057534d2a26c6b5ebc1bfeb9631a635d3d4616831308233a64720b57124b61e3b4625a5a58c4e4d438ab5d744de2f75f3fa2451544529397789f8c4a6440d6b32c40d85390f95a84335c8a20123c075809fc8b52b2fa16678474c5982abeeb74e2063c216a4cf8ea9eed9f48104748ce560ba5d203e049397a23ca7bac5b955e5610379ee39c0e859d3113010e2a41e374b3dce2e2399ebcff4558ef5f65d9d0d451fe4e9a3aaf3d18a831e0bf8896b60841c92a52d5276ee565985d7958ba5ddf7fda161ec9a1bccf21faf17929f9d3cd9b29272bad0bc72b156db6bac2f30b54681056044ef570773c9c232ee12416e37829202c133e994c6f6d0958d01f391be28f57825b32a1751c906c68f605ef94a01666b5ad32a43320d5a1de1069049437bc96842ffb0b36abcafa04e0e1af284b22377e9fe468fe5f55684731c17e84c29e7337d6edd08e5ca91a75b3da37d61b1c5d8b7688d0b8730e7391ecef17ab1a82b338ddbd55f1d60d1272548ff56461b0822925a8cfade8c76b34ee77be244f255761d0a38405b07dca975e3734985e33b53676d92d7adc51701c39f7e9d17f79db90942b26bbf25f8ae786c31a601ee601a63dfcae3bbb524140bf9966506a37be3fd12f05c78b00a54d0cf72b4f99d463ceb2162fd2a9535bb1e9d3ea7349b4df6b80501c9aac6c18ef97a630214b9550847ca8fc25a316e3d9a8eb1a11277d5640f50831609d8f83d9c518418abe168512bae1eb763218b5ffc5e4951938dccd36ab32ae92196b436a18d5a95b3c9c2de698afa07ba2432dd66069f2dbe544704b04e19af1021e3fb3ec20d13611c90d89f887c7e0b56bd38edde1d4f0bba513b5b71e8a95f10f127b30892406e2842267a117ee4bf9246a611a0127701f21afdbb1376daf32dfd97429e183879f5c752dcd01208f2f99b310543ee96827a22b771dd6fbc39bf78ec11b57ce29dd89edbb7029282ebbfd1617c880f52d31675773f9433b317c1b112f2cbc4be0e04eaf53c041b773a0ad8d30b9bf8a572e5267687aa36aa9adad201f01f25dd2c93cde5e170360d76704ed4485a3dc30b757892543811cfeebb71faa397ead153ae36a2460779e2565f6844efa25bb1d43d575ec958de2c57a33a31874c50193429b8eb24390fc886766ce593c53d14e6dca03af66357d988a91f7dde4feee3fe99d9332cc64eeedbeb3601fcd0d3c3eb7d87cea45d15df1015be7910e30ec42116e80375fe7fdbe1b7a0f98fab17154ef2b9cb74b6d3bab22466d727ff4e23de14e3025d01c3279004a1d017521e052475dcdaca87ad1a2fdfdadbe7d918732cb6f3cdeac6299549bd4b225405d36325f7d98a8bc070a118526033e8d153c9ffd316b036f091300a999504446746c9b7e633de130b7218d9763675d02cdcd4bc5a3dcf5f3f5ecf99b8a78320106a613b8e5c35cde2f331f6e7a566880e575e8e1d5abef3cd5f73cefe5861ddaf59dfc1956eefcda3390b2b997d598c47dda576c145787f1744e09a3587");
    block->nVersion = 1;
    block->vtx.push_back(MakeTransactionRef(std::move(txNew)));
    block->hashPrevBlock = uint256S("0x111000000000000000000000000000000000000000000000000000000000ff11");
    block->hashMerkleRoot = BlockMerkleRoot(*block);
    return block;
}

BOOST_FIXTURE_TEST_SUITE(authorization_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(no_authorization)
{
    CScript script = CScript() << 0 << CScriptNum(4);
    CBlock *block = CreateBlock(script);

    CBlockIndex* block_index = new CBlockIndex();
    block_index->nHeight = 15;
    chainActive.SetTip(block_index);

    CAuthTestParams params;

    bool authorized = CheckAuthorization(block, params);
    BOOST_CHECK_EQUAL(authorized, true);
}

BOOST_AUTO_TEST_CASE(height_lt_fork)
{
    CScript script = CScript() << 0 << CScriptNum(4);
    CBlock *block = CreateBlock(script);

    CBlockIndex* block_index = new CBlockIndex();
    block_index->nHeight = 15676;
    chainActive.SetTip(block_index);

    CAuthTestParams params(15678, "67de5101fed5e846d9093f84c2fc4f8a49d32f40f83c325893c06287413c63dc");

    bool authorized = CheckAuthorization(block, params);
    BOOST_CHECK_EQUAL(authorized, true);
}

BOOST_AUTO_TEST_CASE(no_auth_pubkey)
{
    CScript script = CScript() << 0;
    CBlock *block = CreateBlock(script);

    CBlockIndex* block_index = new CBlockIndex();
    block_index->nHeight = 15677;
    chainActive.SetTip(block_index);

    CAuthTestParams params(15678);

    bool authorized = CheckAuthorization(block, params);
    BOOST_CHECK_EQUAL(authorized, true);
}

BOOST_AUTO_TEST_CASE(invalid_script_size)
{
    CScript script = CScript() << 0;
    CBlock *block = CreateBlock(script);

    CBlockIndex* block_index = new CBlockIndex();
    block_index->nHeight = 15677;
    chainActive.SetTip(block_index);

    CAuthTestParams params(15678, "67de5101fed5e846d9093f84c2fc4f8a49d32f40f83c325893c06287413c63dc");

    bool authorized = CheckAuthorization(block, params);
    BOOST_CHECK_EQUAL(authorized, false);
}

BOOST_AUTO_TEST_CASE(invalid_signature)
{

    CScript script = CScript() << 15678 << time(nullptr) << ParseHex("ff0c1419e5b97fbd3e2d8ccc431b9effccc9a3f88fefe693ed4055612867cc472e7731770adeb26e4260bd4cf8095525d66120d110c805d1edaa37d22d013d09");
    CBlock* block = CreateBlock(script);

    CBlockIndex* block_index = new CBlockIndex();
    block_index->nHeight = 15677;
    chainActive.SetTip(block_index);

    CAuthTestParams params(15678, "67de5101fed5e846d9093f84c2fc4f8a49d32f40f83c325893c06287413c63dc");

    bool authorized = CheckAuthorization(block, params);
    BOOST_CHECK_EQUAL(authorized, false);
}

BOOST_AUTO_TEST_CASE(valid_signature)
{
    CScript script = CScript() << 15678 << time(nullptr) << ParseHex("df0c1419e5b97fbd3e2d8ccc431b9effccc9a3f88fefe693ed4055612867cc472e7731770adeb26e4260bd4cf8095525d66120d110c805d1edaa37d22d013d09");
    CBlock* block = CreateBlock(script);

    CBlockIndex* block_index = new CBlockIndex();
    block_index->nHeight = 15677;
    chainActive.SetTip(block_index);

    CAuthTestParams params(15678, "67de5101fed5e846d9093f84c2fc4f8a49d32f40f83c325893c06287413c63dc");

    bool authorized = CheckAuthorization(block, params);
    BOOST_CHECK_EQUAL(authorized, true);
}

BOOST_AUTO_TEST_SUITE_END()