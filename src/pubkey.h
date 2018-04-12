// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PUBKEY_H
#define BITCOIN_PUBKEY_H

#include <hash.h>
#include <serialize.h>
#include <uint256.h>

#include <stdexcept>
#include <vector>

// 1 byte: depth: 0x00 for master nodes, 0x01 for level-1 derived keys, ....
// 4 bytes: the fingerprint of the parent's key (0x00000000 if master key)
// 4 bytes: child number
// 32 bytes: the chain code
// 32 bytes: the public key
const unsigned int BIP32_EXT_PUBKEY_SIZE = 73;

/** A reference to a CKey: the Hash160 of its serialized public key */
class CKeyID : public uint160
{
public:
    CKeyID() : uint160() {}
    explicit CKeyID(const uint160& in) : uint160(in) {}
};

typedef uint256 ChainCode;

/** An encapsulated public key. */
class CPubKey
{
public:
    /**
     * ed25519:
     */
    static const unsigned int PUBLIC_KEY_SIZE             = 32;
    static const unsigned int SIGNATURE_SIZE              = 64;

private:

    unsigned char vch[PUBLIC_KEY_SIZE];

public:
    //! Construct an invalid public key.
    CPubKey()
    {
    }

    //! Initialize a public key using begin/end iterators to byte data.
    template <typename T>
    void Set(const T pbegin, const T pend)
    {
        if (size_t(pend - pbegin) == PUBLIC_KEY_SIZE)
            memcpy(vch, (unsigned char*)&pbegin[0], PUBLIC_KEY_SIZE);
    }

    //! Construct a public key using begin/end iterators to byte data.
    template <typename T>
    CPubKey(const T pbegin, const T pend)
    {
        Set(pbegin, pend);
    }

    //! Construct a public key from a byte vector.
    explicit CPubKey(const std::vector<unsigned char>& _vch)
    {
        Set(_vch.begin(), _vch.end());
    }

    //! Simple read-only vector-like interface to the pubkey data.
    unsigned int size() const { return PUBLIC_KEY_SIZE; }
    const unsigned char* begin() const { return vch; }
    const unsigned char* end() const { return vch + size(); }
    const unsigned char& operator[](unsigned int pos) const { return vch[pos]; }

    //! Comparator implementation.
    friend bool operator==(const CPubKey& a, const CPubKey& b)
    {
        return a.vch[0] == b.vch[0] &&
               memcmp(a.vch, b.vch, a.size()) == 0;
    }
    friend bool operator!=(const CPubKey& a, const CPubKey& b)
    {
        return !(a == b);
    }
    friend bool operator<(const CPubKey& a, const CPubKey& b)
    {
        return a.vch[0] < b.vch[0] ||
               (a.vch[0] == b.vch[0] && memcmp(a.vch, b.vch, a.size()) < 0);
    }

    //! Implement serialization, as if this was a byte vector.
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        unsigned int len = size();
        ::WriteCompactSize(s, len);
        s.write((char*)vch, len);
    }
    template <typename Stream>
    void Unserialize(Stream& s)
    {
        unsigned int len = ::ReadCompactSize(s);
        if (len == PUBLIC_KEY_SIZE) {
            s.read((char*)vch, len);
        } else {
            // invalid pubkey, skip available data
            char dummy;
            while (len--)
                s.read(&dummy, 1);
        }
    }

    //! Get the KeyID of this public key (hash of its serialization)
    CKeyID GetID() const
    {
        return CKeyID(Hash160(vch, vch + size()));
    }

    //! Get the 256-bit hash of this public key.
    uint256 GetHash() const
    {
        return Hash(vch, vch + size());
    }

    /*
     * Check syntactic correctness.
     *
     * Note that this is consensus critical as CheckSig() calls it!
     */
    bool IsValid() const
    {
        return size() > 0;
    }

    //! fully validate whether this is a valid public key (more expensive than IsValid())
    bool IsFullyValid() const;

    /**
     * Verify a signature
     * If this public key is not fully valid, the return value will be false.
     */
    bool Verify(const uint256& hash, const std::vector<unsigned char>& vchSig) const;

    // ed25519的签名没有延展性问题，所以无需检测S

    // ed25519没有ecdsa这种从签名恢复公钥的特性，所以删除RecoverCompact

    //! Derive BIP32 child pubkey.
    bool Derive(CPubKey& pubkeyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;
};

struct CExtPubKey {
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    ChainCode chaincode;
    CPubKey pubkey;

    friend bool operator==(const CExtPubKey &a, const CExtPubKey &b)
    {
        return a.nDepth == b.nDepth &&
            memcmp(&a.vchFingerprint[0], &b.vchFingerprint[0], sizeof(vchFingerprint)) == 0 &&
            a.nChild == b.nChild &&
            a.chaincode == b.chaincode &&
            a.pubkey == b.pubkey;
    }

    void Encode(unsigned char code[BIP32_EXT_PUBKEY_SIZE]) const;
    void Decode(const unsigned char code[BIP32_EXT_PUBKEY_SIZE]);
    bool Derive(CExtPubKey& out, unsigned int nChild) const;

    void Serialize(CSizeComputer& s) const
    {
        // Optimized implementation for ::GetSerializeSize that avoids copying.
        s.seek(BIP32_EXT_PUBKEY_SIZE + 1); // add one byte for the size (compact int)
    }
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        unsigned int len = BIP32_EXT_PUBKEY_SIZE;
        ::WriteCompactSize(s, len);
        unsigned char code[BIP32_EXT_PUBKEY_SIZE];
        Encode(code);
        s.write((const char *)&code[0], len);
    }
    template <typename Stream>
    void Unserialize(Stream& s)
    {
        unsigned int len = ::ReadCompactSize(s);
        unsigned char code[BIP32_EXT_PUBKEY_SIZE];
        if (len != BIP32_EXT_PUBKEY_SIZE)
            throw std::runtime_error("Invalid extended key size\n");
        s.read((char *)&code[0], len);
        Decode(code);
    }
};

/** Users of this module must hold an ECCVerifyHandle. The constructor and
 *  destructor of these are not allowed to run in parallel, though. */
class ECCVerifyHandle
{
    static int refcount;

public:
    ECCVerifyHandle();
    ~ECCVerifyHandle();
};

#endif // BITCOIN_PUBKEY_H
