
cmake_policy(SET CMP0020 NEW)
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DENABLE_WALLET)

set(CMAKE_AUTOMOC OFF)

file(GLOB QT_TRANS *.ts)

include_directories(../..)

qt5_add_translation(LOCALE_FILES ${QT_TRANS})

set_source_files_properties(${LOCALE_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})

add_library(rcoin-qt-locale STATIC ${QT_TRANS} ${LOCALE_FILES} dummy.cpp)
