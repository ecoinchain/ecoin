// Copyright (c) 2013-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>

#include <key.h>
#include <key_io.h>
#include <uint256.h>
#include <util.h>
#include <utilstrencodings.h>
#include <test/test_bitcoin.h>

#include <string>
#include <vector>

struct TestDerivation {
    std::string pub;
    std::string prv;
    unsigned int nChild;
};

struct TestVector {
    std::string strHexMaster;
    std::vector<TestDerivation> vDerive;

    explicit TestVector(std::string strHexMasterIn) : strHexMaster(strHexMasterIn) {}

    TestVector& operator()(std::string pub, std::string prv, unsigned int nChild) {
        vDerive.push_back(TestDerivation());
        TestDerivation &der = vDerive.back();
        der.pub = pub;
        der.prv = prv;
        der.nChild = nChild;
        return *this;
    }
};

TestVector test1 =
  TestVector("000102030405060708090a0b0c0d0e0f")
    ("xpuboDwANUFfqnAWJXMPUvFVWR8PYZek3EdrSyUgwo8ouPgwKMZChBuYTDHeDG2VXg5NPjaTTCYhggaddrXEMnFhEqvGgATE4ZhpnLnNjbsnkb",
     "xprvZpKzXX6CgXVFJ1CTyHDh9491ZxvHLSV3zFTvKeGHqU8rgAE1iPLsWoQiKCwNjsFuMJ8FmvD7MXtBSYiG7L1JvyUFYeBRuXNoDgGEovu5XoHDpAbEZGpnpoeo8cduMhNVs3dEwSaaphC6akEEGpNrgk",
     0x80000000) // harden child
    ("xpuboEZffC8m9eHsnGiVRy2WvkHxCpwbzqKw4GaQXEQe3Lmky8SUersYt1JewV7bs1jiWasomDD3D6qzU4RjpoRVEb9J7gnSUXGg3t6kCPhtMi",
     "xprvZpWqVYjUgD8pGkTgL9VdAdRtdYk7wARpwLtE2D8DYUXMW1hE4SNpdzCjiyyAw7j8iEfNFfX3hRxhMJZFQfkhtSYM4AxfGTKoSxDFHdTeoJpopyhwfFmtCpmDfhLJkdtdKSsNtS62MgBowp9KJGhKAr",
     1) // normal child
    ("xpuboEyfVsMeedB22u6zcCGBgkfEkhDwEtasZTuZ6Z2cVDoMAZ5xszmqmcdd7WWTx56vf3fQJmuVJAqJSakpxaWXF9yG21C63CAJ7qymYpaeaQ",
     "xprvZpdxxNz1Jrvtn9GqqtdsvKasCDAKSabdAtjU43ckCiN83cLwVH8ZR24PYmeiqWRsKNvkr3Ea57sPhTZV8pe7t7JAETTWUZ3UsQwV4myXLfYoW2ttDL6gyVyGkBbne8KGK6j7Xat1B7cDgdDizkN6yr",
     0x80000002) // harden child
    ("xpuboFFu5kYMTgJ2qQRVk9a15xEENyP3Ei34WKSStJpqMzSUneWMG4iPHg5pMo3UkMxDc1KQJ1LaT2ceH4p7NkKBfYLJ9CVtk9qtdE2zTjhH9r",
     "xprvZpinfVp7BkjswthXmSs4gtzjqgfFD8NEnj3UhagUmJLmiBSCH82F4QK8ejLWAJ6TfafHKZNWEE6jVjZxUKjBUBGv4cuqTPPc5X4xyS1fBN9YSTJu5wCZJXNZukGjA3dVyK8zM93d69RYCr2tzd5GGU",
     2) // normal child
    ("xpuboFr2VYqpkE44husyiVEetHXwLH3ASVn8Bzc9DZrLNH89bE2JvxKGwVtrd967ZQPemiEbib3r3VvhVGYD1ma2xMNarKbyWhcKeGFGyyPY3y",
     "xprvZptvfwfcaoLMhpxBVpti8eaZnCH8wS3xz53MVKKKEm6ekQ94u37L2vWFRqLj2mktbXfUUMJoMyzYuhCB6AvP1farD3sAuskPWCygKuxBnwSdnJ2tDGb2MeW1vD7b9LBUBix5qryDMaDdEYksPxd6Pn",
     1000000000) // normal child
    ("xpuboGTrLzF5YBzEmzNwZjADL1e1VGyVsXYtXV3Frstbwy9hqBhnAdD18YXdnjcVKXbBRfnqWZfZcnwNas9tQDtf82ajWoeE7Jh9wiAJJ43APg",
     "xprvZq5ZvWKaGvdUSRDk9ge7NzjfrEEmKcpfFEi75UWxVUkq6y9LP1czeQdcEB1R6Z4eaQ4DDGGYCQMaCEMU95f7ZGcmBkekpGvxfYTUbrHvHzHfZWCsnBxMFTJZZTbjaCFL4GcZfrFdXmDfqPc2boz8wA",
     0); // normal child

TestVector test2 =
  TestVector("fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542")
    ("xpuboDwANUFfqnAWJSFoJ5ZECnpDBJxWbmxcnN5yKm377iJ2NJAN2FhyUKfkcKjfPoNF4LmdoZJVqTTT4QuTsA46AbjnFmKwvYnKdKQESqEgFi",
     "xprvZpKzXX6CgXVFHygf5pMWvNdZf6zrvVp8qgQzE2CmJPWmFrjZ5iSVwGTDavrR9AMYQBESCufXixyQbp9L8nBhAF2MFcxogCaSV86ZYmnNiPtwHwJkuwLabpisSRb6oh16ZU4jFaZhgr5YR5HQ4X3Abh",
     0)
    ("xpuboEky1VbLho2ZywLPqHwZpDjLWvfCDjbfj74Ka1LD1j3qyUipkPBX3tibiuyVsW66fXmtYo8Njkc8FtG1m6ncMQW5t47ZgCcQZx6245B1uZ",
     "xprvZpaCAxyNXUkyw7jvBC4rf2k1d7sqU7pEEJsjbwtBVhakaCAk6Ga6ZvjV55joLUCzCKXQ62gDgzUZif8vX8teSGSUtAfiJJFskGuWjrsSiK19Y2DTx6zS3bHvWvUgAW7p6eWanTTpK5XxgN1VDLFKeq",
     0xFFFFFFFF)
    ("xpuboEnyrTLZRiwXNwVa5E5fSwvQgHsT7WPaXMAuANVn3Qz1t5d3LeR8rmLgKDa2NkPc4cUvHtnwua7yYFV5nno3rj6MZ7N1Fnde69t4AsK2uo",
     "xprvZpant9J2y2HiRQhDjCPaWx9PFHEyojr6UmpUhydgaAG3tPVVdbHE9PVDbWd1a1KptjuPtuQFSjvRHD8ujWhwSQKegGQNZYz3STprBmvTA8UYidijo6D7e4WucHVrp1x6US9KRuC5c7SWgtpDKjLzc3",
     1)
    ("xpuboFa2qR3ML5gTT996ufXBdEBHKuE8Xif9TCAYHqJtQKoMT5RndGHUHTYNSVuUvjrZQdRchHGoBB9EZ6uPfQWJH5LfyoV34AvccEeBhp767b",
     "xprvZppB6n1bTASUGssjZwC91mjQoSLNk1kkMRhVH5BFZgDUTvvu2CiSNrMNg34CzDy4FiqKo9dtWTwNwgu78gYZs8SYp5Cwd7gFhSWVXgvvJkqaWmy9GDBp5iom6XXsd4vhbi4sDef7wpxnZ544Uyu5V8",
     0xFFFFFFFE)
    ("xpuboG2RLkYmD24vS9hN7rxochR9ZTmUjRX4nGKX8LyZ1WfThJRHmUUcUa2akgJ4rAPzKB8SnVBMCpKthx8nfnwSJwUhA8yBenWhGLjg6cKshr",
     "xprvZpx1kn4wQ6YveA6x2MTSF3VfG8WyQk7YmDTbSA2iEhzG8uPhKny2kBdpdzTq8ssBS2thcttxGcLFepnofgQ9DcMhtv7ooL7iz3v1KJpjEAQJcrFbbCyYRaMuZcWp1rqouH9mzGJLig3W7VUmFsHB2Q",
     2)
    ("xpuboG5kooh39WLDKS5M3D7aEEqXBpzDPjCSYbCxXwEVmggJNSrwi5utdS2KwADEQmotabeQ4Bi3pkvieDiYxN3AMrrSpnJA62erKd9P8J2zsi",
     "xprvZpy1E8LAj1vyRBr5GkPJXtpgwKf3QgnzzSXam412mMamEFQMLmDzvCKnb5mM3cSmoCoEV1Y4YCHYBGCNeuXH3M6xED3m3Ap9fEkPkhh5fstyaa2UG34Wnv6HkRTzWsbJfnSGKKapNyh2GLcKF6wz65",
     0);

TestVector test3 =
  TestVector("4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba45d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be")
    ("xpuboDwANUFfqnAWJDtzWrXbYj2E4Cv3MhTuUzji1UWCAMvTHNHuDAKmE7G5mSywL2198jwRYMUyY7j6Fo2B6S1LkS5Szondk26z6PM4Ceujys",
     "xprvZpKzXX6CgXVFHv1io3DMNoxDasACxTumXQrba1289gJR2KNXWNgwqbrjGBaVcSgDV9N1cvYVbubtfWXENiAUkxSk5ANHoDJ166UbWQETAL4L6mdvSBdvTJGuhkdctu5ASh21ZUeMFr8EeeVVhepyvs",
      0x80000000)
    ("xpuboEgmyXW86dxN5Cf8hRdNTCgxPqGYPPqWZFxYZkYVXbWHHMyJs1C9A44FeWu85xyVHk9vFTL5u56Bqh5Uhby1jWX8SkdmXSgHbvtWCssCRP",
     "xprvZpYwyb7aum7x8sNPLuydZQvMFCUwBJ5uBnzMvVCLeAGQYzLMq5Uh6pw84Y3mnPxiFbU2SBz6yfAswrjSvJnSVwwASPsXCn69pezs4RPgUcGsjcWuXAqpE5J9Kw1Kn5CvLwwqGokjkHpV46WSWGnFUM",
      0);

void RunTest(const TestVector &test) {
    std::vector<unsigned char> seed = ParseHex(test.strHexMaster);
    CExtKey key;
    CExtPubKey pubkey;
    key.SetMaster(seed.data(), seed.size());
    pubkey = key.Neuter();
    for (const TestDerivation &derive : test.vDerive) {
        // Test private key
        BOOST_CHECK(EncodeExtKey(key) == derive.prv);
        BOOST_CHECK(DecodeExtKey(derive.prv) == key); //ensure a base58 decoded key also matches

        // Test public key
        BOOST_CHECK(EncodeExtPubKey(pubkey) == derive.pub);
        BOOST_CHECK(DecodeExtPubKey(derive.pub) == pubkey); //ensure a base58 decoded pubkey also matches

        // Derive new keys
        CExtKey keyNew;
        BOOST_CHECK(key.Derive(keyNew, derive.nChild));
        CExtPubKey pubkeyNew = keyNew.Neuter();
        if (!(derive.nChild & 0x80000000)) {
            // Compare with public derivation
            CExtPubKey pubkeyNew2;
            BOOST_CHECK(pubkey.Derive(pubkeyNew2, derive.nChild));
            BOOST_CHECK(pubkeyNew == pubkeyNew2);
        }
        key = keyNew;
        pubkey = pubkeyNew;

        CDataStream ssPub(SER_DISK, CLIENT_VERSION);
        ssPub << pubkeyNew;
        BOOST_CHECK(ssPub.size() == BIP32_EXT_PUBKEY_SIZE + 1);

        CDataStream ssPriv(SER_DISK, CLIENT_VERSION);
        ssPriv << keyNew;
        BOOST_CHECK(ssPriv.size() == BIP32_EXT_PRIVKEY_SIZE + 1);

        CExtPubKey pubCheck;
        CExtKey privCheck;
        ssPub >> pubCheck;
        ssPriv >> privCheck;

        BOOST_CHECK(pubCheck == pubkeyNew);
        BOOST_CHECK(privCheck == keyNew);
    }
}

BOOST_FIXTURE_TEST_SUITE(bip32_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(bip32_test1) {
    RunTest(test1);
}

BOOST_AUTO_TEST_CASE(bip32_test2) {
    RunTest(test2);
}

BOOST_AUTO_TEST_CASE(bip32_test3) {
    RunTest(test3);
}

BOOST_AUTO_TEST_SUITE_END()
