
include(CheckIncludeFile)

include(CheckSymbolExists)
CHECK_INCLUDE_FILE(endian.h HAVE_ENDIAN_H)

if (HAVE_ENDIAN_H)
	add_definitions(-DHAVE_ENDIAN_H=1)
endif()

CHECK_SYMBOL_EXISTS(htobe16 "endian.h" HAVE_DECL_HTOBE16)

if (HAVE_DECL_HTOBE16)
	add_definitions(-DHAVE_DECL_HTOBE16)
endif()

CHECK_SYMBOL_EXISTS(htole16 "endian.h" HAVE_DECL_HTOLE16)

if (HAVE_DECL_HTOLE16)
	add_definitions(-DHAVE_DECL_HTOLE16)
endif()

CHECK_SYMBOL_EXISTS(be16toh "endian.h" HAVE_DECL_BE16TOH)

if (HAVE_DECL_BE16TOH)
	add_definitions(-DHAVE_DECL_BE16TOH)
endif()

CHECK_SYMBOL_EXISTS(le16toh "endian.h" HAVE_DECL_LE16TOH)

if (HAVE_DECL_LE16TOH)
	add_definitions(-DHAVE_DECL_LE16TOH)
endif()

CHECK_SYMBOL_EXISTS(htobe32 "endian.h" HAVE_DECL_HTOBE32)
if (HAVE_DECL_HTOBE32)
	add_definitions(-DHAVE_DECL_HTOBE32)
endif()

CHECK_SYMBOL_EXISTS(htole32 "endian.h" HAVE_DECL_HTOLE32)
if (HAVE_DECL_HTOLE32)
	add_definitions(-DHAVE_DECL_HTOLE32)
endif()
CHECK_SYMBOL_EXISTS(be32toh "endian.h" HAVE_DECL_BE32TOH)
if (HAVE_DECL_BE32TOH)
	add_definitions(-DHAVE_DECL_BE32TOH)
endif()
CHECK_SYMBOL_EXISTS(le32toh "endian.h" HAVE_DECL_LE32TOH)
if (HAVE_DECL_LE32TOH)
	add_definitions(-DHAVE_DECL_LE32TOH)
endif()

CHECK_SYMBOL_EXISTS(htobe64 "endian.h" HAVE_DECL_HTOBE64)
if (HAVE_DECL_HTOBE64)
	add_definitions(-DHAVE_DECL_HTOBE64)
endif()
CHECK_SYMBOL_EXISTS(htole64 "endian.h" HAVE_DECL_HTOLE64)
if (HAVE_DECL_HTOLE64)
	add_definitions(-DHAVE_DECL_HTOLE64)
endif()
CHECK_SYMBOL_EXISTS(be64toh "endian.h" HAVE_DECL_BE64TOH)
if (HAVE_DECL_BE64TOH)
	add_definitions(-DHAVE_DECL_BE64TOH)
endif()
CHECK_SYMBOL_EXISTS(le64toh "endian.h" HAVE_DECL_LE64TOH)
if (HAVE_DECL_LE64TOH)
	add_definitions(-DHAVE_DECL_LE64TOH)
endif()

CHECK_SYMBOL_EXISTS(MSG_DONTWAIT "sys/socket.h" HAVE_MSG_DONTWAIT)
if (HAVE_MSG_DONTWAIT)
	add_definitions(-DHAVE_MSG_DONTWAIT)
endif()

CHECK_SYMBOL_EXISTS(htobe16 "endian.h" HAVE_DECL_HTOBE16)
if (HAVE_DECL_HTOBE16)
	add_definitions(-DHAVE_DECL_HTOBE16)
endif()

CHECK_SYMBOL_EXISTS(htole16 "endian.h" HAVE_DECL_HTOLE16)
if (HAVE_DECL_HTOLE16)
	add_definitions(-DHAVE_DECL_HTOLE16)
endif()
CHECK_SYMBOL_EXISTS(be16toh "endian.h" HAVE_DECL_BE16TOH)
if (HAVE_DECL_BE16TOH)
	add_definitions(-DHAVE_DECL_BE16TOH)
endif()


CHECK_SYMBOL_EXISTS(le16toh "endian.h" HAVE_DECL_LE16TOH)
if (HAVE_DECL_LE16TOH)
	add_definitions(-DHAVE_DECL_LE16TOH)
endif()

CHECK_SYMBOL_EXISTS(htobe32 "endian.h" HAVE_DECL_HTOBE32)
if (HAVE_DECL_HTOBE32)
	add_definitions(-DHAVE_DECL_HTOBE32)
endif()
CHECK_SYMBOL_EXISTS(htole32 "endian.h" HAVE_DECL_HTOLE32)
if (HAVE_DECL_HTOLE32)
	add_definitions(-DHAVE_DECL_HTOLE32)
endif()
CHECK_SYMBOL_EXISTS(be32toh "endian.h" HAVE_DECL_BE32TOH)
if (HAVE_DECL_BE32TOH)
	add_definitions(-DHAVE_DECL_BE32TOH)
endif()
CHECK_SYMBOL_EXISTS(le32toh "endian.h" HAVE_DECL_LE32TOH)
if (HAVE_DECL_LE32TOH)
	add_definitions(-DHAVE_DECL_LE32TOH)
endif()
CHECK_SYMBOL_EXISTS(htobe64 "endian.h" HAVE_DECL_HTOBE64)
if (HAVE_DECL_HTOBE64)
	add_definitions(-DHAVE_DECL_HTOBE64)
endif()
CHECK_SYMBOL_EXISTS(htole64 "endian.h" HAVE_DECL_HTOLE64)
if (HAVE_DECL_HTOLE64)
	add_definitions(-DHAVE_DECL_HTOLE64)
endif()
CHECK_SYMBOL_EXISTS(be64toh "endian.h" HAVE_DECL_BE64TOH)
if (HAVE_DECL_BE64TOH)
	add_definitions(-DHAVE_DECL_BE64TOH)
endif()
CHECK_SYMBOL_EXISTS(le64toh "endian.h" HAVE_DECL_LE64TOH)
if (HAVE_DECL_LE64TOH)
	add_definitions(-DHAVE_DECL_LE64TOH)
endif()


CHECK_SYMBOL_EXISTS(bswap_16 "byteswap.h" HAVE_DECL_BSWAP_16)
if (HAVE_DECL_BSWAP_16)
	add_definitions(-DHAVE_DECL_BSWAP_16)
endif()

CHECK_SYMBOL_EXISTS(bswap_32 "byteswap.h" HAVE_DECL_BSWAP_32)
if (HAVE_DECL_BSWAP_32)
	add_definitions(-DHAVE_DECL_BSWAP_32)
endif()

CHECK_SYMBOL_EXISTS(bswap_64 "byteswap.h" HAVE_DECL_BSWAP_64)
if (HAVE_DECL_BSWAP_64)
	add_definitions(-DHAVE_DECL_BSWAP_64)
endif()

## Add solvers here
add_definitions(-DUSE_CPU_TROMP)
message("-- USE_CPU_TROMP DEFINED")

add_subdirectory(cpu_tromp)

option(USE_CUDA_DJEZO "USE CUDA_DJEZO" OFF)

option(USE_OCL_SILENTARMY "use opencl solver" OFF)

if(USE_CUDA_DJEZO)
add_definitions(-DUSE_CUDA_DJEZO)
message("-- USE_CUDA_DJEZO DEFINED")
add_subdirectory(cuda_djezo)
endif()

if(USE_OCL_SILENTARMY)
add_definitions(-DUSE_OCL_SILENTARMY)
message("-- USE_OCL_SILENTARMY DEFINED")
add_subdirectory(ocl_silentarmy)
endif()

add_subdirectory(leveldb EXCLUDE_FROM_ALL)
add_subdirectory(univalue)
add_subdirectory(secp256k1)
add_subdirectory(ed25519)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DCLIENT_VERSION_MAJOR=0 -DCLIENT_VERSION_MINOR=17
	-DCLIENT_VERSION_REVISION=3 -DCLIENT_VERSION_BUILD=0
        -DCLIENT_VERSION_IS_RELEASE=1 -DCOPYRIGHT_YEAR=2018)

add_definitions(-DENABLE_WALLET)

link_libraries(libsodium-static ${DB_LIBRARY} ${OPENSSL_LIBRARIES} ${PROTOBUF_LIBRARY} univalue leveldb)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${DB_INCLUDE_DIR})

add_definitions(-DPACKAGE_NAME="rcoin")

add_library(concore_crypto STATIC   crypto/aes.cpp
  crypto/aes.h
  crypto/chacha20.h
  crypto/chacha20.cpp
  crypto/common.h
  crypto/equihash.cpp
  crypto/equihash.h
  crypto/equihash.tcc
  crypto/hmac_sha256.cpp
  crypto/hmac_sha256.h
  crypto/hmac_sha512.cpp
  crypto/hmac_sha512.h
  crypto/ripemd160.cpp
  crypto/ripemd160.h
  crypto/sha1.cpp
  crypto/sha1.h
  crypto/sha256.cpp
  crypto/sha256.h
  crypto/sha3_256.cpp
  crypto/sha3_256.h
  crypto/sha512.cpp
  crypto/sha512.h
  blake2/blake2bx.cpp
)

add_library(coincore STATIC
  base58.cpp
  bech32.cpp
  chainparams.cpp
  coins.cpp
  compressor.cpp
  core_read.cpp
  core_write.cpp
  key.cpp
  key_io.cpp
  keystore.cpp
  netaddress.cpp
  netbase.cpp
  policy/feerate.cpp
  protocol.cpp
  scheduler.cpp
  script/ismine.cpp
  script/sign.cpp
  script/standard.cpp
  warnings.cpp


    amount.h
  arith_uint256.cpp
  arith_uint256.h
  consensus/merkle.cpp
  consensus/merkle.h
  consensus/params.h
  consensus/validation.h
  hash.cpp
  hash.h
  prevector.h
  primitives/block.cpp
  primitives/block.h
  primitives/transaction.cpp
  primitives/transaction.h
  pubkey.cpp
  pubkey.h
  script/bitcoinconsensus.cpp
  script/interpreter.cpp
  script/interpreter.h
  script/script.cpp
  script/script.h
  script/script_error.cpp
  script/script_error.h
  serialize.h
  tinyformat.h
  uint256.cpp
  uint256.h
  utilstrencodings.cpp
  utilstrencodings.h
  version.h

  support/lockedpool.cpp
  chainparamsbase.cpp
  clientversion.cpp
  compat/glibc_sanity.cpp
  compat/glibcxx_sanity.cpp
#  compat/strnlen.cpp
  fs.cpp
  random.cpp
  rpc/protocol.cpp
  rpc/util.cpp
  support/cleanse.cpp
  sync.cpp
  threadinterrupt.cpp
  util.cpp
  utilmoneystr.cpp
  utilstrencodings.cpp
  utiltime.cpp

  rpc/client.cpp

  MinerFactory.cpp

  addrdb.cpp
  addrman.cpp
  bloom.cpp
  blockencodings.cpp
  chain.cpp
  checkpoints.cpp
  consensus/tx_verify.cpp
  httprpc.cpp
  httpserver.cpp
  init.cpp
  dbwrapper.cpp
  merkleblock.cpp
  miner.cpp
  net.cpp
  net_processing.cpp
  noui.cpp
  policy/fees.cpp
  policy/policy.cpp
  policy/rbf.cpp
  pow.cpp
  rest.cpp
  rpc/blockchain.cpp
  rpc/mining.cpp
  rpc/misc.cpp
  rpc/net.cpp
  rpc/rawtransaction.cpp
  rpc/safemode.cpp
  rpc/server.cpp
  script/sigcache.cpp
  timedata.cpp
  torcontrol.cpp
  txdb.cpp
  txmempool.cpp
  ui_interface.cpp
  validation.cpp
  validationinterface.cpp
  versionbits.cpp

  )

if(WIN32)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rd-src/libevent-master/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rd-src/libevent-master/WIN32-Code/nmake)
endif()

target_link_libraries(coincore concore_crypto secp256k1 ed25519)

if(WIN32)
	target_link_libraries(coincore event_extra_static event_core_static)
else()
	target_link_libraries(coincore ${EV_LIBRARIES})
endif()

# link libs
target_link_libraries(coincore cpu_tromp)
if(USE_CUDA_DJEZO)
target_link_libraries(coincore cuda_djezo)
endif()

if(USE_OCL_SILENTARMY)
target_link_libraries(coincore ocl_silentarmy)
endif()

add_subdirectory(wallet)
add_subdirectory(qt)

add_executable(rcoind bitcoind.cpp)
add_executable(rcoin-cli bitcoin-cli.cpp)

target_link_libraries(rcoind concore_crypto coincore wallet)
target_link_libraries(rcoin-cli concore_crypto coincore wallet)
target_link_libraries(rcoind libsodium-static ${DB_LIBRARY} ${OPENSSL_LIBRARIES} univalue leveldb qrencode)
target_link_libraries(rcoin-cli libsodium-static ${DB_LIBRARY} ${OPENSSL_LIBRARIES} univalue leveldb qrencode)
target_link_libraries(rcoind ${Boost_LIBRARIES} )
target_link_libraries(rcoin-cli ${Boost_LIBRARIES})
