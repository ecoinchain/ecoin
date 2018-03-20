import os
from itertools import islice
import random

from test_framework.base58 import b58encode_chk, b58decode_chk, b58chars
from test_framework.script import CScript, OP_HASH160, OP_CHECKSIG, hash160, OP_EQUAL, OP_DUP, OP_EQUALVERIFY

# 为key_io_tests.cpp准备测试数据
# python test/functional/feature_address.py valid 50 > src/test/data/key_io_valid.json

# key types
PUBKEY_ADDRESS = 33
SCRIPT_ADDRESS = 15
PUBKEY_ADDRESS_TEST = 111
SCRIPT_ADDRESS_TEST = 196
PRIVKEY = 128
PRIVKEY_TEST = 239

metadata_keys = ['isPrivkey', 'chain', 'addrType', 'isCompressed']
# templates for valid sequences
templates = [
  # prefix, payload_size, suffix, metadata
  #                                  None = N/A
  ((PUBKEY_ADDRESS,),      20, (),   (False, 'main', 'pubkey', None)),
  ((SCRIPT_ADDRESS,),      20, (),   (False, 'main', 'script',  None)),
  ((PUBKEY_ADDRESS_TEST,), 20, (),   (False, 'test',  'pubkey', None)),
  ((SCRIPT_ADDRESS_TEST,), 20, (),   (False, 'test',  'script',  None)),
  ((PRIVKEY,),             32, (),   (True,  'main', None,  False)),
  ((PRIVKEY,),             32, (1,), (True,  'main', None,  True)),
  ((PRIVKEY_TEST,),        32, (),   (True,  'test',  None,  False)),
  ((PRIVKEY_TEST,),        32, (1,), (True,  'test',  None,  True))
]

def is_valid(v):
    '''Check vector v for validity'''
    result = b58decode_chk(v)
    if result is None:
        return False   
    for template in templates:
        prefix = bytearray(template[0])
        suffix = bytearray(template[2])
        if result.startswith(prefix) and result.endswith(suffix):
            if (len(result) - len(prefix) - len(suffix)) == template[1]:
                return True
    return False

def gen_valid_vectors():
    '''Generate valid test vectors'''
    while True:
        for template in templates:
            prefix = bytearray(template[0])
            payload = os.urandom(template[1])
            suffix = bytearray(template[2])
            chk_b = bytearray()
            chk_b.extend(prefix)
            chk_b.extend(payload)
            chk_b.extend(suffix)
            metadata = dict([(x,y) for (x,y) in zip(metadata_keys,template[3]) if y is not None])
            rv = b58encode_chk(chk_b)
            assert is_valid(rv)
            if (metadata['isPrivkey']):
                yield (rv, payload.hex(), metadata)
            else:
                if (metadata['addrType'] == 'pubkey'):
                    # OP_DUP << OP_HASH160 << ToByteVector(keyID) << OP_EQUALVERIFY << OP_CHECKSIG;
                    p2pkh = CScript([OP_DUP, OP_HASH160, payload, OP_EQUALVERIFY, OP_CHECKSIG])
                    yield (rv, p2pkh.hex(), metadata)
                else:
                    # OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
                    p2sh = CScript([OP_HASH160, payload, OP_EQUAL])	
                    yield (rv, p2sh.hex(), metadata)

def gen_invalid_vector(template, corrupt_prefix, randomize_payload_size, corrupt_suffix):
    '''Generate possibly invalid vector'''
    if corrupt_prefix:
        prefix = os.urandom(1)
    else:
        prefix = bytearray(template[0])
    
    if randomize_payload_size:
        payload = os.urandom(max(int(random.expovariate(0.5)), 50))
    else:
        payload = os.urandom(template[1])
    
    if corrupt_suffix:
        suffix = os.urandom(len(template[2]))
    else:
        suffix = bytearray(template[2])

    chk_b = bytearray()
    chk_b.extend(prefix)
    chk_b.extend(payload)
    chk_b.extend(suffix)
    return b58encode_chk(chk_b)

def randbool(p = 0.5):
    '''Return True with P(p)'''
    return random.random() < p

def gen_invalid_vectors():
    '''Generate invalid test vectors'''
    # start with some manual edge-cases
    yield "",
    yield "x",
    while True:
        # kinds of invalid vectors:
        #   invalid prefix
        #   invalid payload length
        #   invalid (randomized) suffix (add random data)
        #   corrupt checksum
        for template in templates:
            val = gen_invalid_vector(template, randbool(0.2), randbool(0.2), randbool(0.2))
            if random.randint(0,10)<1: # line corruption
                if randbool(): # add random character to end
                    val += random.choice(b58chars)
                else: # replace random character in the middle
                    n = random.randint(0, len(val))
                    val = val[0:n] + random.choice(b58chars) + val[n+1:]
            if not is_valid(val):
                yield val,

if __name__ == '__main__':
    import sys, json
    iters = {'valid':gen_valid_vectors, 'invalid':gen_invalid_vectors}
    try:
        uiter = iters[sys.argv[1]]
    except IndexError:
        uiter = gen_valid_vectors
    try:
        count = int(sys.argv[2])
    except IndexError:
        count = 0
   
    data = list(islice(uiter(), count))
    json.dump(data, sys.stdout, sort_keys=True, indent=4)
    sys.stdout.write('\n')

