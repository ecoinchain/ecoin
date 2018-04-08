
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(asn1)
include_directories(evp)
include_directories(modes)

file(GLOB SRCS
	aes/*.c
	asn1/*.c
	bf/*.c
	bio/*.c
	bn/*.c
	buffer/*.c
	camellia/*.c
	cast/*.c
	cmac/*.c
	cms/*.c
	conf/*.c
	dh/*.c

	des/*.c


	dsa/*.c
	dso/*.c
	ec/*.c
	ecdsa/*.c
	ecdh/*.c
	err/*.c
	evp/*.c
	hmac/*.c
	idea/*.c
	lhash/*.c
	md5/*.c
	modes/*.c
	objects/*.c
	ocsp/*.c
	pem/*.c
	pqueue/*.c
	pkcs7/*.c
	pkcs12/*.c
	rand/*.c
	rc4/*.c
	rsa/*.c
	seed/*.c
	sha/*.c
	srp/*.c
	stack/*.c
	ts/*.c
	txt_db/*.c
	ui/*.c
	x509/*.c
	x509v3/*.c
	o_init.c o_dir.c o_str.c o_time.c ex_data.c cpt_err.c cryptlib.c mem.c mem_dbg.c mem_clr.c  cversion.c)

add_library(crypto_static STATIC ${SRCS})

set_target_properties(crypto_static PROPERTIES FOLDER "third_party/")

target_compile_definitions(crypto_static PUBLIC
	-DOPENSSL_NO_ASM
	-DOPENSSL_NO_ENGINE
	-DOPENSSL_NO_RC1
	-DOPENSSL_NO_RC2
	-DOPENSSL_NO_RC3
	-DOPENSSL_NO_MD1
	-DOPENSSL_NO_MD2
	-DOPENSSL_NO_MD3
	-DOPENSSL_NO_MD4
	-DOPENSSL_NO_MDC2
	-DOPENSSL_NO_WHIRLPOOL
	-DOPENSSL_NO_RIPEMD
	-DOPENSSL_NO_COMP
	-DOPENSSL_NO_OCSP
	-DWIN32_LEAN_AND_MEAN )

target_include_directories(crypto_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include/)
