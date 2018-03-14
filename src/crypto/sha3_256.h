// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CRYPTO_SHA3_256_H
#define BITCOIN_CRYPTO_SHA3_256_H

#include <stdint.h>
#include <stdlib.h>
#include <array>

/** A hasher class for SHA3-256. */
class CSHA3_256
{
private:
    std::array<uint64_t, 25> A;
	std::array<unsigned char, 144> m;
	size_t pos;
	uint64_t total;

public:
    static const size_t OUTPUT_SIZE = 32;
    static const size_t RATE = 1600U - 2 * 256;

    CSHA3_256();
    CSHA3_256& Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CSHA3_256& Reset();
};

#endif // BITCOIN_CRYPTO_SHA3_256_H
