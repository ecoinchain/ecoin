cmake_minimum_required(VERSION 3.0)
cmake_minimum_required(VERSION 3.0)
project(rcoin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=c++14 HAS_CXX14)
if (HAS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")
endif()

option(ENABLE_QT_STATIC "link to static qt" ON)
option(Boost_USE_STATIC_LIBS "use static Boost" ON)
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")

if(MSVC)
set (CMAKE_CONFIGURATION_TYPES "Release;Debug")
set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_MinSizeRel
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_MinSizeRel
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

endif(MSVC)

find_package(Threads)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

if(MSVC AND ENABLE_QT_STATIC)
	# For static Windows builds, we also need to pull in the Qt5 Platform
	# support library, which is not exposed to CMake properly, unfortunately
	GET_TARGET_PROPERTY(QT_LIB_DIR "${Qt5Widgets_LIBRARIES}" LOCATION)	
	GET_FILENAME_COMPONENT(QT_LIB_DIR "${QT_LIB_DIR}" PATH)

	message(STATUS "using qt in ${QT_LIB_DIR}")

	FIND_LIBRARY(Qt5QICO_LIBRARIES_RELEASE qico
		HINTS "${QT_LIB_DIR}/../plugins/imageformats"
	)
	FIND_LIBRARY(Qt5QICO_LIBRARIES_DEBUG qicod
		HINTS "${QT_LIB_DIR}/../plugins/imageformats"
	)

	FIND_LIBRARY(Qt5HB_LIBRARIES_RELEASE qtharfbuzzng
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5HB_LIBRARIES_DEBUG qtharfbuzzngd
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
	SET(Qt5QICO_LIBRARIES optimized ${Qt5QICO_LIBRARIES_RELEASE} debug ${Qt5QICO_LIBRARIES_DEBUG})
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
	set(Qt5StaticExtra ${Qt5QICO_LIBRARIES} ${Qt5HB_LIBRARIES} ${Qt5PNG_LIBRARIES} ${Qt5FT_LIBRARIES} ${Qt5PCRE_LIBRARIES})
	link_libraries(${Qt5QICO_LIBRARIES} ${Qt5PNG_LIBRARIES} ${Qt5HB_LIBRARIES} ${Qt5FT_LIBRARIES} ${Qt5PCRE_LIBRARIES})
endif()

add_subdirectory(3rd-src/openssl-cmake)

if(WIN32)
	add_definitions(-DBOOST_ALL_STATIC_LINK -DBOOST_AUTO_LINK_NOMANGLE)
	add_definitions(-DBOOST_THREAD_USE_LIB)
	add_definitions(-DBOOST_FILESYSTEM_STATIC_LINK)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
endif()

link_directories(${QT_LIB_DIR})
add_subdirectory(3rd-src/boost)
include_directories(3rd-src/boost)

add_subdirectory(3rd-src/protobuf)
find_package(Protobuf)

set(DISABLE_SSP 1)
set(DISABLE_ASM 1)

add_subdirectory(3rd-src/libsodium EXCLUDE_FROM_ALL)

add_subdirectory(3rd-src/db-4.8.30 EXCLUDE_FROM_ALL)

add_subdirectory(3rd-src/libevent-master EXCLUDE_FROM_ALL)

add_subdirectory(src)
