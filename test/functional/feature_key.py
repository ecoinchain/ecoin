from test_framework.address import key_to_p2pkh, key_to_p2sh_p2wpkh
from test_framework.key import CECKey
import secrets
import sys
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad
import scrypt
import json
import leveldb
from multiprocessing import Queue
import threading
import plyvel

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
	address = key_to_p2sh_p2wpkh(pk, is_main)
	iv = secrets.token_bytes(AES.block_size)
	# 这里的salt, key的size以及n, p, r的值跟bitcoinj中的KeyCrypterScrypt定义一致
	key = scrypt.hash(convert_to_byte_array(password.encode("utf-8")), salt, 16384, 8, 1, 32)
	obj = AES.new(key, AES.MODE_CBC, iv)
	# 加密数据必须是16位的整数倍，采用pkcs7进行pad，跟PaddedBufferedBlockCipher中的一致
	ciphertext = obj.encrypt(pad(wif.encode('utf-8'), 16))
	key_info = {
		'pubkey': pk.hex(),
		'wif': wif,
		'address': address,
		'salt': salt.hex(),
		'n': n,
		'p': p,
		'r': r,
		'iv': iv.hex(),
		'cipher': ciphertext.hex()
	}
	return key_info

#生产者类
class Producer(threading.Thread):
	def __init__(self, name,queue,wif_version,is_main,password,num):
		threading.Thread.__init__(self, name=name)
		self.data=queue
		self.wif_version=wif_version
		self.is_main=is_main
		self.password=password
		self.num=num

	def run(self):
		for i in range(self.num):
			key_info = single_key(self.wif_version, self.is_main, self.password)
			self.data.put(key_info)
			if (i > 0 and i % 100 == 0):
				print("%s produce %d" % (self.getName(), i))
		print("%s finished!" % self.getName())

#消费者类
class Consumer(threading.Thread):
	def __init__(self,name,queue,db,num):
		threading.Thread.__init__(self,name=name)
		self.data=queue
		self.db=db
		self.num=num
	def run(self):
		wb = self.db.write_batch()
		for i in range(self.num):
			key_info = self.data.get()
			address = key_info['address']
			wb.put(address.encode('utf-8'), json.dumps(key_info).encode('utf-8'))
			if (i > 0 and i % 100 == 0):
				print("%s consume %d" % (self.getName(), i))
		wb.write()
		print("%s finished!" % self.getName())

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
		thread = int(sys.argv[7])
		db = plyvel.DB(db_path, create_if_missing=True)
		queue = Queue()
		consumer = Consumer('Consumer', queue, db, num * thread)
		consumer.start()
		producers = []
		for i in range(thread):
			producer = Producer('Producer-' + str(i), queue, wif_version, is_main, password, num)
			producers.append(producer)
			producer.start()
		for producer in producers:
			producer.join()
		consumer.join()
		db.close()
		print('生成成功')
	elif func == 'leveldb_show':
		db_path = sys.argv[2]
		db = leveldb.LevelDB(db_path)
		keys = list(db.RangeIter())
		print(keys)
