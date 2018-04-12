// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <compressor.h>

#include <hash.h>
#include <pubkey.h>
#include <script/standard.h>

bool CScriptCompressor::IsToKeyID(CKeyID &hash) const
{
    if (script.size() == 25 && script[0] == OP_DUP && script[1] == OP_HASH160
                            && script[2] == 20 && script[23] == OP_EQUALVERIFY
                            && script[24] == OP_CHECKSIG) {
        memcpy(&hash, &script[3], 20);
        return true;
    }
    return false;
}

bool CScriptCompressor::IsToScriptID(CScriptID &hash) const
{
    if (script.size() == 23 && script[0] == OP_HASH160 && script[1] == 20
                            && script[22] == OP_EQUAL) {
        memcpy(&hash, &script[2], 20);
        return true;
    }
    return false;
}

bool CScriptCompressor::IsToPubKey(CPubKey &pubkey) const
{
    // pubkey长度固定在32
    if (script.size() == CPubKey::PUBLIC_KEY_SIZE + 2 && script[0] == CPubKey::PUBLIC_KEY_SIZE && script[CPubKey::PUBLIC_KEY_SIZE + 1] == OP_CHECKSIG) {
        pubkey.Set(&script[1], &script[CPubKey::PUBLIC_KEY_SIZE + 1]);
        return true;
    }
    return false;
}

bool CScriptCompressor::Compress(std::vector<unsigned char> &out) const
{
    CKeyID keyID;
    if (IsToKeyID(keyID)) {
        out.resize(21);
        out[0] = 0x00;
        memcpy(&out[1], &keyID, 20);
        return true;
    }
    CScriptID scriptID;
    if (IsToScriptID(scriptID)) {
        out.resize(21);
        out[0] = 0x01;
        memcpy(&out[1], &scriptID, 20);
        return true;
    }
    CPubKey pubkey;
    if (IsToPubKey(pubkey)) {
        out.resize(CPubKey::PUBLIC_KEY_SIZE + 1);
        out[0] = 0x02;
        memcpy(&out[1], &pubkey[0], CPubKey::PUBLIC_KEY_SIZE);
        return true;
    }
    return false;
}

unsigned int CScriptCompressor::GetSpecialSize(unsigned int nSize) const
{
    if (nSize == 0 || nSize == 1)
        return 20;
    if (nSize == 2)
        return CPubKey::PUBLIC_KEY_SIZE;
    return 0;
}

bool CScriptCompressor::Decompress(unsigned int nSize, const std::vector<unsigned char> &in)
{
    switch(nSize) {
    case 0x00:
        script.resize(25);
        script[0] = OP_DUP;
        script[1] = OP_HASH160;
        script[2] = 20;
        memcpy(&script[3], in.data(), 20);
        script[23] = OP_EQUALVERIFY;
        script[24] = OP_CHECKSIG;
        return true;
    case 0x01:
        script.resize(23);
        script[0] = OP_HASH160;
        script[1] = 20;
        memcpy(&script[2], in.data(), 20);
        script[22] = OP_EQUAL;
        return true;
    case 0x02:
        script.resize(CPubKey::PUBLIC_KEY_SIZE + 2);
        script[0] = CPubKey::PUBLIC_KEY_SIZE;
        memcpy(&script[1], in.data(), CPubKey::PUBLIC_KEY_SIZE);
        script[CPubKey::PUBLIC_KEY_SIZE + 1] = OP_CHECKSIG;
        return true;
    }
    return false;
}

// Amount compression:
// * If the amount is 0, output 0
// * first, divide the amount (in base units) by the largest power of 10 possible; call the exponent e (e is max 9)
// * if e<9, the last digit of the resulting number cannot be 0; store it as d, and drop it (divide by 10)
//   * call the result n
//   * output 1 + 10*(9*n + d - 1) + e
// * if e==9, we only know the resulting number is not zero, so output 1 + 10*(n - 1) + 9
// (this is decodable, as d is in [1-9] and e is in [0-9])

uint64_t CTxOutCompressor::CompressAmount(uint64_t n)
{
    if (n == 0)
        return 0;
    int e = 0;
    while (((n % 10) == 0) && e < 9) {
        n /= 10;
        e++;
    }
    if (e < 9) {
        int d = (n % 10);
        assert(d >= 1 && d <= 9);
        n /= 10;
        return 1 + (n*9 + d - 1)*10 + e;
    } else {
        return 1 + (n - 1)*10 + 9;
    }
}

uint64_t CTxOutCompressor::DecompressAmount(uint64_t x)
{
    // x = 0  OR  x = 1+10*(9*n + d - 1) + e  OR  x = 1+10*(n - 1) + 9
    if (x == 0)
        return 0;
    x--;
    // x = 10*(9*n + d - 1) + e
    int e = x % 10;
    x /= 10;
    uint64_t n = 0;
    if (e < 9) {
        // x = 9*n + d - 1
        int d = (x % 9) + 1;
        x /= 9;
        // x = n
        n = x*10 + d;
    } else {
        n = x+1;
    }
    while (e) {
        n *= 10;
        e--;
    }
    return n;
}
