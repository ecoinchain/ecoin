// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_KEY_H
#define BITCOIN_KEY_H

#include <pubkey.h>
#include <serialize.h>
#include <support/allocators/secure.h>
#include <uint256.h>

#include <stdexcept>
#include <vector>
#include <ed25519/ed25519.h>

// 1 byte: depth: 0x00 for master nodes, 0x01 for level-1 derived keys, ....
// 4 bytes: the fingerprint of the parent's key (0x00000000 if master key)
// 4 bytes: child number
// 32 bytes: the chain code
// 64 bytes: private key
const unsigned int BIP32_EXT_PRIVKEY_SIZE = 105;

/**
 * secure_allocator is defined in allocators.h
 * CPrivKey is a serialized private key, with all parameters included
 * (PRIVATE_KEY_SIZE bytes)
 */
typedef std::vector<unsigned char, secure_allocator<unsigned char> > CPrivKey;

/** An encapsulated private key. */
class CKey
{
public:
    /**
     * ed25519:
     */
    static const unsigned int SEED_SIZE                   = 32;
    static const unsigned int PRIVATE_KEY_SIZE            = 64;

private:
    //! Whether this private key is valid. We check for correctness when modifying the key
    //! data, so fValid should always correspond to the actual state.
    bool fValid;

    CPrivKey privkey;

public:
    //! Construct an invalid private key.
    CKey() : fValid(false)
    {
        privkey.resize(PRIVATE_KEY_SIZE);
    }

    friend bool operator==(const CKey& a, const CKey& b)
    {
        return a.size() == b.size() &&
            memcmp(a.privkey.data(), b.privkey.data(), a.size()) == 0;
    }

    //! Initialize using begin and end iterators to byte data.
    template <typename T>
    void SetSeed(const T pbegin, const T pend)
    {
        if (size_t(pend - pbegin) != SEED_SIZE) {
            fValid = false;
        } else {
            unsigned char seed[SEED_SIZE];
            memcpy(seed, (unsigned char*)&pbegin[0], SEED_SIZE);
            ed25519_create_privkey(privkey.data(), seed);
            fValid = true;
        }
    }

    template <typename T>
    void SetPrivKey(const T pbegin, const T pend)
    {
        if (size_t(pend - pbegin) != PRIVATE_KEY_SIZE) {
            fValid = false;
        } else {
            memcpy(privkey.data(), (unsigned char*)&pbegin[0], PRIVATE_KEY_SIZE);
            fValid = true;
        }
    }

    //! Simple read-only vector-like interface.
    unsigned int size() const { return PRIVATE_KEY_SIZE; }
    const unsigned char* begin() const { return privkey.data(); }
    const unsigned char* end() const { return privkey.data() + size(); }

    //! Check whether this private key is valid.
    bool IsValid() const { return fValid; }

    //! Generate a new private key using a cryptographic PRNG.
    void MakeNewKey(bool fCompressed);

    /**
     * Convert the private key to a CPrivKey (serialized OpenSSL private key data).
     * This is expensive.
     */
    CPrivKey GetPrivKey() const;

    /**
     * Compute the public key from a private key.
     * This is expensive.
     */
    CPubKey GetPubKey() const;

    /**
     * Create a DER-serialized signature.
     * The test_case parameter tweaks the deterministic nonce.
     */
    bool Sign(const uint256& hash, std::vector<unsigned char>& vchSig, uint32_t test_case = 0) const;

    // ed25519没有ecdsa这种从签名恢复公钥的特性，所以删除SignCompact

    //! Derive BIP32 child key.
    bool Derive(CKey& keyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;

    /**
     * Verify thoroughly whether a private key and a public key match.
     * This is done using a different mechanism than just regenerating it.
     */
    bool VerifyPubKey(const CPubKey& vchPubKey) const;

    //! Load private key and check that public key matches.
    bool Load(const CPrivKey& privkey, const CPubKey& vchPubKey, bool fSkipCheck);
};

struct CExtKey {
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    ChainCode chaincode;
    CKey key;

    friend bool operator==(const CExtKey& a, const CExtKey& b)
    {
        return a.nDepth == b.nDepth &&
            memcmp(&a.vchFingerprint[0], &b.vchFingerprint[0], sizeof(vchFingerprint)) == 0 &&
            a.nChild == b.nChild &&
            a.chaincode == b.chaincode &&
            a.key == b.key;
    }

    void Encode(unsigned char code[BIP32_EXT_PRIVKEY_SIZE]) const;
    void Decode(const unsigned char code[BIP32_EXT_PRIVKEY_SIZE]);
    bool Derive(CExtKey& out, unsigned int nChild) const;
    CExtPubKey Neuter() const;
    void SetMaster(const unsigned char* seed, unsigned int nSeedLen);
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        unsigned int len = BIP32_EXT_PRIVKEY_SIZE;
        ::WriteCompactSize(s, len);
        unsigned char code[BIP32_EXT_PRIVKEY_SIZE];
        Encode(code);
        s.write((const char *)&code[0], len);
    }
    template <typename Stream>
    void Unserialize(Stream& s)
    {
        unsigned int len = ::ReadCompactSize(s);
        unsigned char code[BIP32_EXT_PRIVKEY_SIZE];
        if (len != BIP32_EXT_PRIVKEY_SIZE)
            throw std::runtime_error("Invalid extended key size\n");
        s.read((char *)&code[0], len);
        Decode(code);
    }
};

/** Initialize the elliptic curve support. May not be called twice without calling ECC_Stop first. */
void ECC_Start(void);

/** Deinitialize the elliptic curve support. No-op if ECC_Start wasn't called first. */
void ECC_Stop(void);

/** Check that required EC support is available at runtime. */
bool ECC_InitSanityCheck(void);

#endif // BITCOIN_KEY_H
