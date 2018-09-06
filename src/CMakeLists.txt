
include(CheckIncludeFile)

include(CheckSymbolExists)

CHECK_SYMBOL_EXISTS(MSG_DONTWAIT "sys/socket.h" HAVE_MSG_DONTWAIT)
if (HAVE_MSG_DONTWAIT)
	add_definitions(-DHAVE_MSG_DONTWAIT)
endif()

add_subdirectory(leveldb EXCLUDE_FROM_ALL)
add_subdirectory(univalue)
add_subdirectory(secp256k1)

add_subdirectory(blake2)

add_subdirectory(libminer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DCLIENT_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR} -DCLIENT_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR}
	-DCLIENT_VERSION_REVISION=${CPACK_PACKAGE_VERSION_PATCH} -DCLIENT_VERSION_BUILD=0
        -DCLIENT_VERSION_IS_RELEASE=1 -DCOPYRIGHT_YEAR=2018)

add_definitions(-DENABLE_WALLET)

link_libraries(${DB_LIBRARY} ${OPENSSL_LIBRARIES} ${PROTOBUF_LIBRARY} univalue leveldb)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${DB_INCLUDE_DIR})

add_definitions(-DPACKAGE_NAME="ecoin")

set(BITCOIN_CORE_H
  addrdb.h
  addrman.h
  base58.h
  bech32.h
  bloom.h
  blockencodings.h
  chain.h
  chainparams.h
  chainparamsbase.h
  chainparamsseeds.h
  checkpoints.h
  checkqueue.h
  clientversion.h
  coins.h
  compat.h
  compat/sanity.h
  compressor.h
  consensus/consensus.h
  consensus/tx_verify.h
  core_io.h
  core_memusage.h
  cuckoocache.h
  fs.h
  httprpc.h
  httpserver.h
  indirectmap.h
  init.h
  key.h
  key_io.h
  keystore.h
  dbwrapper.h
  limitedmap.h
  memusage.h
  merkleblock.h
  miner.h
  net.h
  net_processing.h
  netaddress.h
  netbase.h
  netmessagemaker.h
  noui.h
  policy/feerate.h
  policy/fees.h
  policy/policy.h
  policy/rbf.h
  pow.h
  protocol.h
  random.h
  reverse_iterator.h
  reverselock.h
  rpc/blockchain.h
  rpc/client.h
  rpc/mining.h
  rpc/protocol.h
  rpc/safemode.h
  rpc/server.h
  rpc/rawtransaction.h
  rpc/register.h
  rpc/util.h
  scheduler.h
  script/ismine.h
  script/sigcache.h
  script/sign.h
  script/standard.h
  streams.h
  support/allocators/secure.h
  support/allocators/zeroafterfree.h
  support/cleanse.h
  support/events.h
  support/lockedpool.h
  sync.h
  threadsafety.h
  threadinterrupt.h
  timedata.h
  torcontrol.h
  txdb.h
  txmempool.h
  ui_interface.h
  undo.h
  util.h
  utilmoneystr.h
  utiltime.h
  validation.h
  validationinterface.h
  versionbits.h
  wallet/coincontrol.h
  wallet/crypter.h
  wallet/db.h
  wallet/feebumper.h
  wallet/fees.h
  wallet/init.h
  wallet/rpcwallet.h
  wallet/wallet.h
  wallet/walletdb.h
  wallet/walletutil.h
  warnings.h
  zmq/zmqabstractnotifier.h
  zmq/zmqconfig.h
  zmq/zmqnotificationinterface.h
  zmq/zmqpublishnotifier.h)

set(libed25519_a_SOURCES
  ed25519/add_scalar.c
  ed25519/ed25519.h
  ed25519/fe.c
  ed25519/fe.h
  ed25519/fixedint.h
  ed25519/ge.c
  ed25519/ge.h
  ed25519/key_exchange.c
  ed25519/keypair.c
  ed25519/precomp_data.h
  ed25519/sc.c
  ed25519/sc.h
  ed25519/seed.c
  ed25519/sha512.c
  ed25519/sha512.h
  ed25519/sign.c
  ed25519/verify.c
)

add_library(bitcoin_crypto STATIC   crypto/aes.cpp
  crypto/aes.cpp
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

add_library(bitcoin_common STATIC
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
  rpc/client.cpp
  ${BITCOIN_CORE_H}
)

add_library(bitcoin_consensus STATIC
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
  ${libed25519_a_SOURCES}
)

add_library(bitcoin_server STATIC
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
  miner.cpp
  LocalClient.cpp
  ${BITCOIN_CORE_H}
)

add_library(bitcoin_util STATIC
  support/lockedpool.cpp
  chainparamsbase.cpp
  clientversion.cpp
  compat/glibc_sanity.cpp
  compat/glibcxx_sanity.cpp
  compat/strnlen.cpp # // NOTE
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
  ${BITCOIN_CORE_H}
)

if(WIN32)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rd-src/libevent-master/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rd-src/libevent-master/WIN32-Code/nmake)
endif()

target_link_libraries(bitcoin_consensus bitcoin_crypto secp256k1)
target_link_libraries(bitcoin_common bitcoin_crypto secp256k1)

if(WIN32)
	target_link_libraries(bitcoin_common event_extra_static event_core_static)
else()
	target_link_libraries(bitcoin_common ${EV_LIBRARIES})
endif()

# link libs
target_link_libraries(bitcoin_server libminer)

add_subdirectory(wallet)
if(WITH_GUI)
add_subdirectory(qt)
endif()

if(NOT WIN32) # do not build command line tools on windows

add_executable(ecoind bitcoind.cpp)
add_executable(ecoin-cli rpc/client.cpp bitcoin-cli.cpp)

target_link_libraries(ecoind bitcoin_crypto bitcoin_crypto bitcoin_common bitcoin_consensus bitcoin_server bitcoin_util wallet)
target_link_libraries(ecoin-cli bitcoin_crypto bitcoin_common bitcoin_consensus bitcoin_server bitcoin_util wallet)
target_link_libraries(ecoind ${DB_LIBRARY} ${OPENSSL_LIBRARIES} univalue leveldb qrencode)
target_link_libraries(ecoin-cli ${DB_LIBRARY} ${OPENSSL_LIBRARIES} univalue leveldb qrencode)
target_link_libraries(ecoind ${Boost_LIBRARIES} )
target_link_libraries(ecoin-cli ${Boost_LIBRARIES})
endif()
