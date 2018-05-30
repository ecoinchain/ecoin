
option(USE_CUDA_DJEZO "USE CUDA_DJEZO" OFF)

option(USE_OCL_SILENTARMY "use opencl solver" OFF)

## Add solvers here
add_definitions(-DUSE_CPU_TROMP)
message("-- USE_CPU_TROMP DEFINED")

add_subdirectory(blake2)

add_subdirectory(cpu_tromp)

if(USE_CUDA_DJEZO)
add_subdirectory(cuda_djezo)
endif()

if(USE_OCL_SILENTARMY)
add_subdirectory(ocl_silentarmy)
endif()

add_library(libminer STATIC MinerFactory.cpp MinerFactory.h speed.cpp libstratum/ZcashStratum.cpp libstratum/StratumClient.cpp)

target_compile_definitions(libminer PUBLIC -DUSE_CPU_TROMP)

target_link_libraries(libminer cpu_tromp)

if(USE_CUDA_DJEZO)
message("-- USE_CUDA_DJEZO DEFINED")
target_compile_definitions(libminer PUBLIC -DUSE_CUDA_DJEZO)
target_link_libraries(libminer cuda_djezo)
endif()

if(USE_OCL_SILENTARMY)
target_compile_definitions(libminer PUBLIC -DUSE_OCL_SILENTARMY)
message("-- USE_OCL_SILENTARMY DEFINED")
target_link_libraries(libminer ocl_silentarmy)
endif()

target_include_directories(libminer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libminer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} .. ../..)
