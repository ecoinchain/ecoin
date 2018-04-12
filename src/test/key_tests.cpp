// Copyright (c) 2012-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <key.h>

#include <key_io.h>
#include <script/script.h>
#include <uint256.h>
#include <util.h>
#include <utilstrencodings.h>
#include <test/test_bitcoin.h>

#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

static const std::string strSecret1 = "2HNAfKjA772BzgmxXGnpRy1ztaiyQuH3MKrZk28GrTS2JWdxXTExS54uieiMqguE4ysV65PFM3YeUdnFJc1k721NwuDJSZX";
static const std::string strSecret2 = "2HEwNb5gwNuPzm4XszQ8vpeEFkF6jSPVUTWJPQuBfrk3buLZDA9grsSF3enDbhkoCeTutLkfEn5SS7p7APiL6DyTAZLKgpW";
static const std::string addr1 = "RJj7jdh8R8gnwKV1u7C7A1cRC3N1qVLoqc";
static const std::string addr2 = "RQfkRi4wFDqfthbyDswEKiGqCDUHCs3yXT";

static const std::string strAddressBad = "1HV9Lc3sNHZxwj4Zk6fB38tEmBryq2cBiF";


BOOST_FIXTURE_TEST_SUITE(key_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(key_test1)
{
    CKey key1  = DecodeKey(strSecret1);
    BOOST_CHECK(key1.IsValid());
    CKey key2  = DecodeKey(strSecret2);
    BOOST_CHECK(key2.IsValid());
    CKey bad_key = DecodeKey(strAddressBad);
    BOOST_CHECK(!bad_key.IsValid());

    CPubKey pubkey1  = key1.GetPubKey();
    CPubKey pubkey2  = key2.GetPubKey();

    BOOST_CHECK(key1.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2));

    BOOST_CHECK(!key2.VerifyPubKey(pubkey1));
    BOOST_CHECK(key2.VerifyPubKey(pubkey2));

    BOOST_CHECK(DecodeDestination(addr1)  == CTxDestination(pubkey1.GetID()));
    BOOST_CHECK(DecodeDestination(addr2)  == CTxDestination(pubkey2.GetID()));

    for (int n=0; n<16; n++)
    {
        std::string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        std::vector<unsigned char> sign1, sign2, sign1C, sign2C;

        BOOST_CHECK(key1.Sign(hashMsg, sign1));
        BOOST_CHECK(key2.Sign(hashMsg, sign2));

        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2));

        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2));
    }

    // test deterministic signing

    std::vector<unsigned char> detsig;
    std::string strMsg = "Very deterministic message";
    uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());
    BOOST_CHECK(key1.Sign(hashMsg, detsig));
    BOOST_CHECK(detsig == ParseHex("ef615c9564302e60dd78d231f88d7c1ea93dda8f9e429303207e79f46bbb14cf9156629af6788dc66da556e2398a067c0fe7dbc8de03c687a46438a0da0cd30a"));
    BOOST_CHECK(key2.Sign(hashMsg, detsig));
    BOOST_CHECK(detsig == ParseHex("1370103ed1c3dbe3787e494e4e3267a4958b07ea135f4fb53e1a7e76b1f6d04e06c2179b4c881cc07236a7f6e83557bb7ec2c4245c1d5117785ed32946e0ac01"));
}

BOOST_AUTO_TEST_SUITE_END()
