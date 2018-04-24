from test_framework.address import key_to_p2pkh
from test_framework.key import CECKey
import secrets
import sys
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad
import scrypt
import json
import leveldb

def print_wif_address(secret):
	k = CECKey()
	k.set_secretbytes(bytes.fromhex(secret))
	pk = k.get_pubkey()
	print(k.get_wif(b'\x80'), key_to_p2pkh(pk, True))

# 对应bitcoinj里面KeyCrypterScrypt.convertToByteArray
def convert_to_byte_array(password):
	cb = []
	for b in password:
		cb.append((b&0xFF00)>>8)
		cb.append(b&0x00FF)
	return bytes(cb)

def single_key(wif_version, is_main, password):
	n = 16384
	p = 1
	r = 8
	seed = secrets.token_bytes(32)
	salt = secrets.token_bytes(8)
	k = CECKey()
	k.set_secretbytes(seed)
	pk = k.get_pubkey()
	wif = k.get_wif(wif_version.to_bytes(1, 'big'))
	address = key_to_p2pkh(pk, is_main)
	iv = secrets.token_bytes(AES.block_size)
	# 这里的salt, key的size以及n, p, r的值跟bitcoinj中的KeyCrypterScrypt定义一致
	key = scrypt.hash(convert_to_byte_array(password.encode("utf-8")), salt, 16384, 8, 1, 32)
	obj = AES.new(key, AES.MODE_CBC, iv)
	# 加密数据必须是16位的整数倍，采用pkcs7进行pad，跟PaddedBufferedBlockCipher中的一致
	ciphertext = obj.encrypt(pad(wif.encode('utf-8'), 16))
	key_info = {
		'address': address,
		'salt': salt.hex(),
		'n': n,
		'p': p,
		'r': r,
		'iv': iv.hex(),
		'cipher': ciphertext.hex()
	}
	return key_info

if __name__ == '__main__':
	func = sys.argv[1]
	if func == 'key_tests':
		# 生成key_tests.cpp所需要的数据
		print_wif_address('F6D814FDEC196FC47F88AC6FF789B687DD5E26F1F98ABF6DE1682631D9F3AB53')
		print_wif_address('29D72592AF765EF8164260A363878F4280C7F71839C5D023E3499DC644088CFA')
	elif func == 'single':
		# 生成单个key信息并显示出来
		wif_version = int(sys.argv[2])
		is_main = sys.argv[3].lower() == 'true'
		password = sys.argv[4]
		key_info = single_key(wif_version, is_main, password)
		print(json.dumps(key_info))
	elif func == 'leveldb':
		# 批量生成多个可以信息并存储到leveldb
		wif_version = int(sys.argv[2])
		is_main = sys.argv[3].lower() == 'true'
		password = sys.argv[4]
		db_path = sys.argv[5]
		num = int(sys.argv[6])
		db = leveldb.LevelDB(db_path)
		for i in range(num):
			key_info = single_key(wif_version, is_main, password)
			address = key_info['address']
			db.Put(address.encode('utf-8'), json.dumps(key_info).encode('utf-8'))
			if (i % 1000 == 0):
				print("i = %d", i)
		print('生成成功')
	elif func == 'leveldb_show':
		db_path = sys.argv[2]
		db = leveldb.LevelDB(db_path)
		keys = list(db.RangeIter())
		print(keys)
