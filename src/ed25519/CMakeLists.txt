
add_library(ed25519 STATIC 
	add_scalar.c 
	ed25519.h 
	fe.c 
	fe.h 
	fixedint.h 
	ge.c 
	ge.h 
	key_exchange.c 
	keypair.c 
	precomp_data.h 
	sc.c 
	sc.h 
	seed.c 
	sha512.c 
	sha512.h 
	sign.c 
	verify.c
)
