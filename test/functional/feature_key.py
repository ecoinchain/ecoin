from test_framework.address import key_to_p2pkh
from test_framework.key import CECKey

def print_wif_address(secret):
	k = CECKey()
	k.set_secretbytes(bytes.fromhex(secret))
	pk = k.get_pubkey()
	print(k.get_wif(b'\x80'), key_to_p2pkh(pk, True))

if __name__ == '__main__':
	# 生成key_tests.cpp所需要的数据
	print_wif_address('F6D814FDEC196FC47F88AC6FF789B687DD5E26F1F98ABF6DE1682631D9F3AB53')
	print_wif_address('29D72592AF765EF8164260A363878F4280C7F71839C5D023E3499DC644088CFA')