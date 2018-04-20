
add_subdirectory(leveldb EXCLUDE_FROM_ALL)

add_subdirectory(univalue)
add_subdirectory(secp256k1)
add_subdirectory(ed25519)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rd-src/libsodium/src/libsodium/include/ ${CMAKE_CURRENT_SOURCE_DIR}../3rd-src/libsodium/src/libsodium/include/sodium)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../3rd-src/libsodium/src/libsodium/include/sodium)

add_definitions(-DCLIENT_VERSION_MAJOR=0 -DCLIENT_VERSION_MINOR=16 -DCLIENT_VERSION_REVISION=99 -DCLIENT_VERSION_BUILD=0
        -DCLIENT_VERSION_IS_RELEASE=1 -DCOPYRIGHT_YEAR=2018)
add_definitions(-DPACKAGE_NAME="rcoin")

add_definitions(-DENABLE_WALLET)

link_libraries(libsodium-static ${DB_LIBRARY} ${OPENSSL_LIBRARIES} ${PROTOBUF_LIBRARY} univalue leveldb)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${DB_INCLUDE_DIR})

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rd-src/libevent-master/include)

if(WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rd-src/libevent-master/WIN32-Code/nmake)
endif()

target_link_libraries(coincore concore_crypto secp256k1 ed25519 event_extra_static event_core_static)

add_subdirectory(wallet)
add_subdirectory(qt)
