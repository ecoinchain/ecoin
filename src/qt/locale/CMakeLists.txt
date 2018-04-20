
cmake_policy(SET CMP0020 NEW)
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DENABLE_WALLET)

file(GLOB QT_TRANS *.ts)

include_directories(../..)

qt5_add_translation(LOCALE_FILES ${QT_TRANS})

set_source_files_properties(${LOCALE_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ecoin-qt-locale OBJECT ${QT_TRANS} ${LOCALE_FILES})
