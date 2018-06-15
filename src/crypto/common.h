// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CRYPTO_COMMON_H
#define BITCOIN_CRYPTO_COMMON_H

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <boost/endian/arithmetic.hpp>
#include <boost/endian/conversion.hpp>

uint16_t static inline ReadLE16(const unsigned char* ptr)
{
	return boost::endian::little_to_native(*reinterpret_cast<const uint16_t*>(ptr));
}

uint32_t static inline ReadLE32(const unsigned char* ptr)
{
	return boost::endian::little_to_native(*reinterpret_cast<const uint32_t*>(ptr));
}

uint64_t static inline ReadLE64(const unsigned char* ptr)
{
	return boost::endian::little_to_native(*reinterpret_cast<const uint64_t*>(ptr));
}

void static inline WriteLE16(unsigned char* ptr, uint16_t x)
{
	*reinterpret_cast<boost::endian::little_uint16_t*>(ptr) = x;
}

void static inline WriteLE32(unsigned char* ptr, uint32_t x)
{
	*reinterpret_cast<boost::endian::little_uint32_t*>(ptr) = x;
}

void static inline WriteLE64(unsigned char* ptr, uint64_t x)
{
	*reinterpret_cast<boost::endian::little_uint64_t*>(ptr) = x;
}

uint32_t static inline ReadBE32(const unsigned char* ptr)
{
	return boost::endian::big_to_native(*reinterpret_cast<const uint32_t*>(ptr));
}

uint64_t static inline ReadBE64(const unsigned char* ptr)
{
	return boost::endian::big_to_native(*reinterpret_cast<const uint64_t*>(ptr));
}

void static inline WriteBE32(unsigned char* ptr, uint32_t x)
{
	*reinterpret_cast<boost::endian::big_uint32_t*>(ptr) = x;
}

void static inline WriteBE64(unsigned char* ptr, uint64_t x)
{
	*reinterpret_cast<boost::endian::big_uint64_t*>(ptr) = x;
}

/** Return the smallest number n such that (x >> n) == 0 (or 64 if the highest bit in x is set. */
uint64_t static inline CountBits(uint64_t x)
{
#if HAVE_DECL___BUILTIN_CLZL
    if (sizeof(unsigned long) >= sizeof(uint64_t)) {
        return x ? 8 * sizeof(unsigned long) - __builtin_clzl(x) : 0;
    }
#endif
#if HAVE_DECL___BUILTIN_CLZLL
    if (sizeof(unsigned long long) >= sizeof(uint64_t)) {
        return x ? 8 * sizeof(unsigned long long) - __builtin_clzll(x) : 0;
    }
#endif
    int ret = 0;
    while (x) {
        x >>= 1;
        ++ret;
    }
    return ret;
}

#endif // BITCOIN_CRYPTO_COMMON_H
