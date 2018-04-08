set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)

cmake_policy(SET CMP0020 NEW)
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DENABLE_WALLET)

file(GLOB SRCS *.cpp qt/*.h)
#file(GLOB UIS forms/*.ui)

include_directories(.. ../../3rd-src/libsodium/src/libsodium/include/ ../../3rd-src/libsodium/src/libsodium/include/sodium)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../3rd-src/libsodium/src/libsodium/include/sodium)
add_subdirectory(forms)

PROTOBUF_GENERATE_CPP(PROTO_PAYMENT_REQUEST_SRC PROTO_PAYMENT_REQUEST_HEADER paymentrequest.proto)

#qt5_wrap_ui(UI_SRCS ${UIS})
qt5_add_resources(RC_SRCS bitcoin.qrc)

set(SRCS ${SRCS} ${UI_SRCS} ${PROTO_PAYMENT_REQUEST_SRC} ${PROTO_PAYMENT_REQUEST_HEADER})

add_executable(rcoin-qt WIN32 ${SRCS} ${RC_SRCS})

target_include_directories(rcoin-qt PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(rcoin-qt rcoin-qt-uis Qt5::Widgets Qt5::Core Qt5::Network)
target_link_libraries(rcoin-qt libsodium-static berkeleydb ${OPENSSL_LIBRARIES} libprotoc)

# target_link_libraries(tianya OPENS)

if(WIN32 AND MSVC AND ENABLE_QT_STATIC)
	target_link_libraries(rcoin-qt ${Qt5StaticExtra})
endif()

install(TARGETS rcoin-qt RUNTIME DESTINATION bin)

add_definitions(-DCLIENT_VERSION_MAJOR=0 -DCLIENT_VERSION_MINOR=1 -DCLIENT_VERSION_REVISION=0 -DCLIENT_VERSION_BUILD=0 -DCLIENT_VERSION_IS_RELEASE=FALSE -DCOPYRIGHT_YEAR=2018)
add_definitions(-DPACKAGE_NAME="rcoin")