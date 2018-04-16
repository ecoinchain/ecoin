// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <key.h>

#include <arith_uint256.h>
#include <crypto/common.h>
#include <crypto/hmac_sha512.h>
#include <random.h>

void CKey::MakeNewKey(bool fCompressedIn) {
    unsigned char seed[SEED_SIZE];
    GetStrongRandBytes(seed, SEED_SIZE);
    ed25519_create_privkey(privkey.data(), seed);
    fValid = true;
}

CPrivKey CKey::GetPrivKey() const {
    assert(fValid);
    return privkey;
}

CPubKey CKey::GetPubKey() const {
    assert(fValid);
    CPubKey pubkey;
    ed25519_privkey_to_pubkey((unsigned char*)pubkey.begin(), privkey.data());
    return pubkey;
}

bool CKey::Sign(const uint256 &hash, std::vector<unsigned char>& vchSig, uint32_t test_case) const {
    if (!fValid)
        return false;
    vchSig.resize(CPubKey::SIGNATURE_SIZE);
    CPubKey pubkey = GetPubKey();
    ed25519_sign(vchSig.data(), hash.begin(), hash.size(), pubkey.begin(), privkey.data());
    return true;
}

bool CKey::VerifyPubKey(const CPubKey& pubkey) const {
    unsigned char rnd[8];
    std::string str = "Bitcoin key verification\n";
    GetRandBytes(rnd, sizeof(rnd));
    uint256 hash;
    CHash256().Write((unsigned char*)str.data(), str.size()).Write(rnd, sizeof(rnd)).Finalize(hash.begin());
    std::vector<unsigned char> vchSig;
    Sign(hash, vchSig);
    return pubkey.Verify(hash, vchSig);
}

bool CKey::Load(const CPrivKey &privkey, const CPubKey &vchPubKey, bool fSkipCheck=false) {
    this->privkey = privkey;
    fValid = true;

    if (fSkipCheck)
        return true;

    return VerifyPubKey(vchPubKey);
}

bool CKey::Derive(CKey& keyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const {
    assert(IsValid());
    unsigned char vout[64];
    if ((nChild >> 31) == 0) {
        // normal child
        CPubKey pubkey = GetPubKey();
        BIP32Hash(cc, nChild, pubkey.begin(), pubkey.size(), vout);
    } else {
        // hardened child
        BIP32Hash(cc, nChild, privkey.data(), privkey.size(), vout);
    }
    memcpy(ccChild.begin(), vout+32, 32);
    // 先将parant privkey和pubkey复制到child，然后child再add一个纯量，直接得到增量之后child privkey和pubkey
    memcpy((unsigned char*)keyChild.begin(), begin(), size());
    ed25519_add_scalar(nullptr, (unsigned char*)keyChild.begin(), vout);
    keyChild.fValid = true;
    return true;
}

bool CExtKey::Derive(CExtKey &out, unsigned int _nChild) const {
    out.nDepth = nDepth + 1;
    CKeyID id = key.GetPubKey().GetID();
    memcpy(&out.vchFingerprint[0], &id, 4);
    out.nChild = _nChild;
    return key.Derive(out.key, out.chaincode, _nChild, chaincode);
}

void CExtKey::SetMaster(const unsigned char *seed, unsigned int nSeedLen) {
    static const unsigned char hashkey[] = {'B','i','t','c','o','i','n',' ','s','e','e','d'};
    std::vector<unsigned char, secure_allocator<unsigned char>> vout(64);
    CHMAC_SHA512(hashkey, sizeof(hashkey)).Write(seed, nSeedLen).Finalize(vout.data());
    // vout的前32个字节用来生成私钥
    unsigned char private_key[CKey::PRIVATE_KEY_SIZE];
    ed25519_create_privkey(private_key, vout.data());
    key.SetPrivKey(private_key, private_key + CKey::PRIVATE_KEY_SIZE);
    memcpy(chaincode.begin(), vout.data() + 32, 32);
    nDepth = 0;
    nChild = 0;
    memset(vchFingerprint, 0, sizeof(vchFingerprint));
}

CExtPubKey CExtKey::Neuter() const {
    CExtPubKey ret;
    ret.nDepth = nDepth;
    memcpy(&ret.vchFingerprint[0], &vchFingerprint[0], 4);
    ret.nChild = nChild;
    ret.pubkey = key.GetPubKey();
    ret.chaincode = chaincode;
    return ret;
}

void CExtKey::Encode(unsigned char code[BIP32_EXT_PRIVKEY_SIZE]) const {
    code[0] = nDepth;
    memcpy(code+1, vchFingerprint, 4);
    code[5] = (nChild >> 24) & 0xFF; code[6] = (nChild >> 16) & 0xFF;
    code[7] = (nChild >>  8) & 0xFF; code[8] = (nChild >>  0) & 0xFF;
    memcpy(code+9, chaincode.begin(), 32);
    assert(key.size() == CKey::PRIVATE_KEY_SIZE);
    memcpy(code+41, key.begin(), CKey::PRIVATE_KEY_SIZE);
}

void CExtKey::Decode(const unsigned char code[BIP32_EXT_PRIVKEY_SIZE]) {
    nDepth = code[0];
    memcpy(vchFingerprint, code+1, 4);
    nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
    memcpy(chaincode.begin(), code+9, 32);
    key.SetPrivKey(code+41, code+BIP32_EXT_PRIVKEY_SIZE);
}

bool ECC_InitSanityCheck() {
    CKey key;
    key.MakeNewKey(true);
    CPubKey pubkey = key.GetPubKey();
    return key.VerifyPubKey(pubkey);
}

void ECC_Start() {
    // 测试生成一对ed25519公私钥
    std::vector<unsigned char, secure_allocator<unsigned char>> vseed(32);
    GetRandBytes(vseed.data(), 32);
    unsigned char public_key[CPubKey::PUBLIC_KEY_SIZE], private_key[CKey::PRIVATE_KEY_SIZE];
    ed25519_create_keypair(public_key, private_key, vseed.data());
}

void ECC_Stop() {
    // do nothing
}
