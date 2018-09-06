
add_library(wallet STATIC
	coincontrol.h
	crypter.cpp
	crypter.h
	db.cpp
	db.h
	feebumper.cpp
	feebumper.h
	fees.cpp
	fees.h
	init.cpp
	init.h
	rpcdump.cpp
	rpcwallet.cpp
	rpcwallet.h
	wallet.cpp
	wallet.h
	walletdb.cpp
	walletdb.h
	walletutil.cpp
	walletutil.h
)

target_include_directories(wallet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(wallet bitcoin_crypto bitcoin_common bitcoin_consensus bitcoin_server bitcoin_util)
