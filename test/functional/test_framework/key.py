# Copyright (c) 2011 Sam Rushing
"""ECC secp256k1 OpenSSL wrapper.

WARNING: This module does not mlock() secrets; your private keys may end up on
disk in swap! Use with caution!

This file is modified from python-bitcoinlib.
"""

import ctypes
import ctypes.util
import hashlib
import sys

from .base58 import b58encode_chk
from binascii import hexlify,unhexlify
from .ed25519 import Ed25519

ssl = ctypes.cdll.LoadLibrary(ctypes.util.find_library ('ssl') or 'libeay32')

ssl.BN_new.restype = ctypes.c_void_p
ssl.BN_new.argtypes = []

ssl.BN_bin2bn.restype = ctypes.c_void_p
ssl.BN_bin2bn.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_void_p]

ssl.BN_bn2bin.restype = ctypes.c_int
ssl.BN_bn2bin.argtypes = [ctypes.c_void_p, ctypes.c_void_p]

ssl.BN_num_bits.restype = ctypes.c_int
ssl.BN_num_bits.argtypes = [ctypes.c_void_p]

ssl.BN_CTX_free.restype = None
ssl.BN_CTX_free.argtypes = [ctypes.c_void_p]

ssl.BN_CTX_new.restype = ctypes.c_void_p
ssl.BN_CTX_new.argtypes = []

ssl.ECDH_compute_key.restype = ctypes.c_int
ssl.ECDH_compute_key.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p]

ssl.ECDSA_sign.restype = ctypes.c_int
ssl.ECDSA_sign.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]

ssl.ECDSA_verify.restype = ctypes.c_int
ssl.ECDSA_verify.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p]

ssl.EC_KEY_free.restype = None
ssl.EC_KEY_free.argtypes = [ctypes.c_void_p]

ssl.EC_KEY_new_by_curve_name.restype = ctypes.c_void_p
ssl.EC_KEY_new_by_curve_name.argtypes = [ctypes.c_int]

ssl.EC_KEY_get0_group.restype = ctypes.c_void_p
ssl.EC_KEY_get0_group.argtypes = [ctypes.c_void_p]

ssl.EC_KEY_get0_public_key.restype = ctypes.c_void_p
ssl.EC_KEY_get0_public_key.argtypes = [ctypes.c_void_p]

ssl.EC_KEY_get0_private_key.restype = ctypes.c_void_p
ssl.EC_KEY_get0_private_key.argtypes = [ctypes.c_void_p]

ssl.EC_KEY_set_private_key.restype = ctypes.c_int
ssl.EC_KEY_set_private_key.argtypes = [ctypes.c_void_p, ctypes.c_void_p]

ssl.EC_KEY_set_conv_form.restype = None
ssl.EC_KEY_set_conv_form.argtypes = [ctypes.c_void_p, ctypes.c_int]

ssl.EC_KEY_set_public_key.restype = ctypes.c_int
ssl.EC_KEY_set_public_key.argtypes = [ctypes.c_void_p, ctypes.c_void_p]

ssl.i2o_ECPublicKey.restype = ctypes.c_void_p
ssl.i2o_ECPublicKey.argtypes = [ctypes.c_void_p, ctypes.c_void_p]

ssl.EC_POINT_new.restype = ctypes.c_void_p
ssl.EC_POINT_new.argtypes = [ctypes.c_void_p]

ssl.EC_POINT_free.restype = None
ssl.EC_POINT_free.argtypes = [ctypes.c_void_p]

ssl.EC_POINT_mul.restype = ctypes.c_int
ssl.EC_POINT_mul.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]

# this specifies the curve used with ECDSA.
NID_secp256k1 = 714 # from openssl/obj_mac.h

SECP256K1_ORDER = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
SECP256K1_ORDER_HALF = SECP256K1_ORDER // 2

# Thx to Sam Devlin for the ctypes magic 64-bit fix.
def _check_result(val, func, args):
    if val == 0:
        raise ValueError
    else:
        return ctypes.c_void_p (val)

ssl.EC_KEY_new_by_curve_name.restype = ctypes.c_void_p
ssl.EC_KEY_new_by_curve_name.errcheck = _check_result

class CECKey():
    
    SEED_SIZE = 32

    def __init__(self):
        return

    def __del__(self):
        return

    def set_secretbytes(self, secret):
        # 种子必须是32位
        if len(secret) != CECKey.SEED_SIZE:
            raise Exception('CKey.set_secretbytes(): secret len must be 32')
        self.privkey,self.pubkey = Ed25519.keygen(secret)
        

    def set_privkey(self, key):
        self.privkey = key

    def set_pubkey(self, key):
        self.pubkey = key

    def get_privkey(self):
        return self.privkey

    def get_pubkey(self):
        return self.pubkey

    def sign(self, msg):
        return Ed25519.sign(self.privkey, self.pubkey, msg)

    def verify(self, msg, sig):
        return Ed25519.verify(self.pubkey, msg, sig) == 1 

    def get_wif(self, version):
        priv_key = bytearray(version)
        priv_key.extend(self.get_privkey())   
        return b58encode_chk(priv_key)    


class CPubKey(bytes):
    """An encapsulated public key

    Attributes:

    is_valid      - Corresponds to CPubKey.IsValid()
    is_fullyvalid - Corresponds to CPubKey.IsFullyValid()
    """

    def __new__(cls, buf, _cec_key=None):
        self = super(CPubKey, cls).__new__(cls, buf)
        if _cec_key is None:
            _cec_key = CECKey()
        self._cec_key = _cec_key
        self.is_fullyvalid = _cec_key.set_pubkey(self) != 0
        return self

    @property
    def is_valid(self):
        return len(self) > 0

    def verify(self, hash, sig):
        return self._cec_key.verify(hash, sig)

    def __str__(self):
        return repr(self)

    def __repr__(self):
        # Always have represent as b'<secret>' so test cases don't have to
        # change for py2/3
        if sys.version > '3':
            return '%s(%s)' % (self.__class__.__name__, super(CPubKey, self).__repr__())
        else:
            return '%s(b%s)' % (self.__class__.__name__, super(CPubKey, self).__repr__())

