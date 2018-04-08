
file(GLOB SRCS *.c)

include_directories(../crypto/)

add_library(eay32_static STATIC ${SRCS})

target_link_libraries(eay32_static crypto_static)

set_target_properties(eay32_static PROPERTIES FOLDER "third_party/")

target_include_directories(eay32_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include/)
