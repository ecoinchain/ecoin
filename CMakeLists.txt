cmake_minimum_required(VERSION 3.0)
project(ecoin)
cmake_policy(SET CMP0058 NEW)
include(CheckCXXCompilerFlag)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(ENABLE_QT_STATIC "link to static qt" ON)
option(Boost_USE_STATIC_LIBS "use static Boost" ON)
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")

if(MSVC)
set (CMAKE_CONFIGURATION_TYPES "Release;Debug")
set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_RELEASE
   )

foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/W3" "/W1" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP")

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}  /ignore:4099 /NODEFAULTLIB:libcmt.lib ")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER}  /SAFESEH:NO ")
else()
check_cxx_compiler_flag(-std=c++14 HAS_CXX14)
if (HAS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")
endif()
endif(MSVC)

FILE(READ "configure.ac" bitcoin_configure_ac_file_content)

string(REGEX MATCH        "define\\(_CLIENT_VERSION_MAJOR, ([0-9]+)\\)"
    _PACKAGE_VERSION_MAJOR_LINE    "${bitcoin_configure_ac_file_content}")

string(REGEX MATCH        "define\\(_CLIENT_VERSION_MINOR, ([0-9]+)\\)"
    _PACKAGE_VERSION_MINOR_LINE    "${bitcoin_configure_ac_file_content}")

string(REGEX MATCH        "define\\(_CLIENT_VERSION_REVISION, ([0-9]+)\\)"
    _PACKAGE_VERSION_PATCH_LINE    "${bitcoin_configure_ac_file_content}")

string(REGEX MATCH        "([0-9]+)"
    CPACK_PACKAGE_VERSION_MAJOR    "${_PACKAGE_VERSION_MAJOR_LINE}")
string(REGEX MATCH        "([0-9]+)"
    CPACK_PACKAGE_VERSION_MINOR    "${_PACKAGE_VERSION_MINOR_LINE}")
string(REGEX MATCH        "([0-9]+)"
    CPACK_PACKAGE_VERSION_PATCH    "${_PACKAGE_VERSION_PATCH_LINE}")

message(STATUS "parsed version is ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")  

include(cpacksettings.cmake)
set(CPACK_DESKTOP_SHORTCUTS ecoin-qt)
INCLUDE(CPack)

find_package(Threads)

link_directories(${CMAKE_THREAD_LIBS_INITD})

list(APPEND RCOIN_QT_COMPONENTS Core Network Gui Svg Widgets LinguistTools UiPlugin)

if (WIN32)
	list (APPEND RCOIN_QT_COMPONENTS WinExtras)
endif(WIN32)
if (APPLE)
	list (APPEND RCOIN_QT_COMPONENTS MacExtras)

	find_library(ObjC objc)
	find_library(CoreFoundation CoreFoundation)
	find_library(IOKit IOKit)
	FIND_LIBRARY(COCOA_LIBRARY Cocoa)
	link_libraries(${CoreFoundation} ${IOKit} ${ObjC} ${COCOA_LIBRARY})
endif (APPLE)

find_package(Qt5 5.9 COMPONENTS ${RCOIN_QT_COMPONENTS} REQUIRED)

if(MSVC AND ENABLE_QT_STATIC)
	# For static Windows builds, we also need to pull in the Qt5 Platform
	# support library, which is not exposed to CMake properly, unfortunately
	GET_TARGET_PROPERTY(QT_LIB_DIR "${Qt5Widgets_LIBRARIES}" LOCATION)
	GET_FILENAME_COMPONENT(QT_LIB_DIR "${QT_LIB_DIR}" PATH)

	message(STATUS "using qt in ${QT_LIB_DIR}")

	FIND_LIBRARY(Qt5HB_LIBRARIES_RELEASE qtharfbuzzng qtharfbuzz
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5HB_LIBRARIES_DEBUG qtharfbuzzngd qtharfbuzzd
		HINTS "${QT_LIB_DIR}"
	)

	FIND_LIBRARY(Qt5PNG_LIBRARIES_RELEASE qtlibpng
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5PNG_LIBRARIES_DEBUG qtlibpngd
		HINTS "${QT_LIB_DIR}"
	)

	FIND_LIBRARY(Qt5FT_LIBRARIES_RELEASE qtfreetype
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5FT_LIBRARIES_DEBUG qtfreetyped
		HINTS "${QT_LIB_DIR}"
	)

	FIND_LIBRARY(Qt5PCRE_LIBRARIES_RELEASE qtpcre2
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5PCRE_LIBRARIES_DEBUG qtpcre2d
		HINTS "${QT_LIB_DIR}"
	)

	SET(Qt5Platform_LIBRARIES optimized ${Qt5Platform_LIBRARIES_RELEASE} debug ${Qt5Platform_LIBRARIES_DEBUG})
	SET(Qt5QWindows_LIBRARIES optimized ${Qt5QWindows_LIBRARIES_RELEASE} debug ${Qt5QWindows_LIBRARIES_DEBUG})
	if (Qt5HB_LIBRARIES_RELEASE)
		SET(Qt5HB_LIBRARIES optimized ${Qt5HB_LIBRARIES_RELEASE} debug ${Qt5HB_LIBRARIES_DEBUG})
	endif()
	if (Qt5PNG_LIBRARIES_RELEASE)
		SET(Qt5PNG_LIBRARIES optimized ${Qt5PNG_LIBRARIES_RELEASE} debug ${Qt5PNG_LIBRARIES_DEBUG})
	endif()
	if (Qt5FT_LIBRARIES_RELEASE)
		SET(Qt5FT_LIBRARIES optimized ${Qt5FT_LIBRARIES_RELEASE} debug ${Qt5FT_LIBRARIES_DEBUG})
	endif()
	if (Qt5PCRE_LIBRARIES_RELEASE)
		SET(Qt5PCRE_LIBRARIES optimized ${Qt5PCRE_LIBRARIES_RELEASE} debug ${Qt5PCRE_LIBRARIES_DEBUG})
	endif()
	ADD_DEFINITIONS(-DSTATIC_QT5)

	set(AS optimized Qt5AccessibilitySupport debug Qt5AccessibilitySupportd)
	set(EDS optimized Qt5EventDispatcherSupport debug Qt5EventDispatcherSupportd)
	set(FDS optimized Qt5FontDatabaseSupport debug Qt5FontDatabaseSupportd)
	set(FBS optimized Qt5FbSupport debug Qt5FbSupportd)
	set(Bsp optimized Qt5Bootstrap debug Qt5Bootstrap)
	set(TS optimized Qt5ThemeSupport debug Qt5ThemeSupportd)

	set(QT_EXTRA_LIBS ${AS} ${EDS} ${FDS} ${FBS} ${Bsp} ${TS})
	set(Qt5StaticExtra ${Qt5HB_LIBRARIES} ${Qt5PNG_LIBRARIES} ${Qt5FT_LIBRARIES} ${Qt5PCRE_LIBRARIES})
	link_libraries(${Qt5PNG_LIBRARIES} ${Qt5HB_LIBRARIES} ${Qt5FT_LIBRARIES} ${Qt5PCRE_LIBRARIES})
endif()

if(WIN32)
add_subdirectory(3rd-src/openssl-cmake)
set(OPENSSL_LIBRARIES crypto_static eay32_static)
else()
find_package(OpenSSL REQUIRED)

add_definitions(-DHAVE_DECL_EVP_MD_CTX_NEW)

endif()

if(WIN32)
	add_definitions(-DBOOST_ALL_STATIC_LINK -DBOOST_AUTO_LINK_NOMANGLE)
	add_definitions(-DBOOST_THREAD_USE_LIB)
	add_definitions(-DBOOST_FILESYSTEM_STATIC_LINK)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
endif()

link_directories(${QT_LIB_DIR})
set(Boost_USE_STATIC_LIBS ON)
add_subdirectory(3rd-src/boost EXCLUDE_FROM_ALL)
include_directories(3rd-src/boost)
set(Boost_LIBRARIES Boost::filesystem Boost::system Boost::thread Boost::date_time Boost::program_options Boost::regex Boost::locale)

set(protobuf_BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
set(protobuf_UNICODE ON)
add_subdirectory(3rd-src/protobuf-3.5.2/cmake)

set(Protobuf_LIBRARY libprotobuf)
set(Protobuf_PROTOC_EXECUTABLE /usr/bin/protoc)
if(MSVC)
set(Protobuf_PROTOC_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/Release/protoc.exe")
endif()

set(Protobuf_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd-src/protobuf-3.5.2/src)
find_package(Protobuf)

set(DISABLE_SSP 1)
set(DISABLE_ASM 1)

if(WIN32)
	add_subdirectory(3rd-src/db-4.8.30 EXCLUDE_FROM_ALL)
	set(DB_LIBRARY berkeleydb)
else()
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	find_package(BerkeleyDB 4.8 REQUIRED)
endif()

if (WIN32)
	add_subdirectory(3rd-src/libevent-master EXCLUDE_FROM_ALL)
else()
	find_package(LIBEVENT)
endif()

add_subdirectory(3rd-src/qrencode)

add_definitions(-DHAVE_DECL_STRNLEN=1)
add_subdirectory(src)
