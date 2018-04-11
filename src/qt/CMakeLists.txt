set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)

cmake_policy(SET CMP0020 NEW)
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DENABLE_WALLET -DUSE_QRCODE)

add_definitions(-DCOPYRIGHT_HOLDERS="rcoin.com")

file(GLOB SRCS *.cpp *.h widgets/*.cpp widgets/*.h)
#file(GLOB UIS forms/*.ui)

add_subdirectory(forms)
add_subdirectory(locale)

PROTOBUF_GENERATE_CPP(PROTO_PAYMENT_REQUEST_SRC PROTO_PAYMENT_REQUEST_HEADER paymentrequest.proto)

#qt5_wrap_ui(UI_SRCS ${UIS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
qt5_add_resources(RC_SRCS bitcoin.qrc bitcoin_locale.qrc)

set(SRCS ${SRCS} ${UI_SRCS} ${PROTO_PAYMENT_REQUEST_SRC} ${PROTO_PAYMENT_REQUEST_HEADER})

add_executable(rcoin-qt WIN32 ${SRCS} ${RC_SRCS})

if(MSVC AND ENABLE_QT_STATIC)
add_definitions(-DQT_STATICPLUGIN)
endif()

target_include_directories(rcoin-qt PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(rcoin-qt rcoin-qt-uis coincore wallet)
target_link_libraries(rcoin-qt Qt5::Widgets Qt5::Gui Qt5::Core Qt5::Network Qt5::Svg ${QT_EXTRA_LIBS})
target_link_libraries(rcoin-qt libsodium-static berkeleydb ${OPENSSL_LIBRARIES} libprotoc univalue leveldb)
target_link_libraries(rcoin-qt Boost::filesystem Boost::system Boost::thread Boost::date_time Boost::program_options )

if(MSVC AND ENABLE_QT_STATIC)
target_link_libraries(rcoin-qt Qt5::QWindowsIntegrationPlugin  Qt5::QSvgPlugin Qt5::QSvgIconPlugin Qt5::QICOPlugin)
endif()

if(WIN32)
target_link_libraries(rcoin-qt Qt5::WinExtras Crypt32.lib Iphlpapi.lib Netapi32.lib Winmm.lib Mincore.lib Imm32.lib Dwmapi.lib)
endif()

# target_link_libraries(tianya OPENS)

if(WIN32 AND MSVC AND ENABLE_QT_STATIC)
	target_link_libraries(rcoin-qt ${Qt5StaticExtra})
endif()

install(TARGETS rcoin-qt RUNTIME DESTINATION bin)

