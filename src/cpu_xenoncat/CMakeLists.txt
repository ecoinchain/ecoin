set(EXECUTABLE cpu_xenoncat)

#cpu_xenoncat/
file(GLOB SRC_LIST
    xenoncat.cpp )
file(GLOB HEADERS
    cpu_xenoncat.hpp
    )


if (USE_CPU_XENONCAT)
	set(AVX1_SRC asm_linux/equihash_avx1.o)
	set(AVX2_SRC asm_linux/equihash_avx2.o)
endif()
    
include_directories(.. ${CMAKE_CURRENT_BINARY_DIR} ${CUDA_INCLUDE_DIRS})

ADD_LIBRARY(${EXECUTABLE} STATIC ${SRC_LIST} ${HEADERS} ${AVX1_SRC} ${AVX2_SRC})


TARGET_LINK_LIBRARIES(${EXECUTABLE} )

install( TARGETS ${EXECUTABLE} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
install( FILES ${HEADERS} DESTINATION include/${EXECUTABLE} )
