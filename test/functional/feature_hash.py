from test_framework.script import hash160
from test_framework.messages import sha256, hash256
from binascii import hexlify,unhexlify

if __name__ == '__main__':
    # 生成script_test中的部分数据
    print(hexlify(sha256(b"")))
    print(hexlify(sha256(b"a")))
    print(hexlify(sha256(b"abcdefghijklmnopqrstuvwxyz")))

    print(hexlify(hash160(b"")))
    print(hexlify(hash160(b"a")))
    print(hexlify(hash160(b"abcdefghijklmnopqrstuvwxyz")))

    print(hexlify(hash256(b"")))
    print(hexlify(hash256(b"a")))
    print(hexlify(hash256(b"abcdefghijklmnopqrstuvwxyz")))

    print(hexlify(hash160(b'\x51')))
    print(hexlify(hash160(b'\xb9')))
    print(hexlify(hash160(b'\x50')))
    print(hexlify(hash160(b'\x62')))
    print(hexlify(hash160(b'\x63\x51\x68')))
    print(hexlify(hash160(b'\x64\x51\x68')))

    print(hexlify(sha256(unhexlify("635168"))))
    print(hexlify(hash160(unhexlify("0020db42be9d58213fee35c471713a4a1de4c1a8654f58008c14836e9a5274278f4d"))))

    print(hexlify(sha256(unhexlify("00"))))
    print(hexlify(sha256(unhexlify("645168"))))
    print(hexlify(hash160(unhexlify("0020fa0b46b1b36a4ef492738c9e0fa744bdb7658838b67c9017e6c2fca79a1ed656"))))