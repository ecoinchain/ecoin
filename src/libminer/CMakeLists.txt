
option(USE_OCL_SILENTARMY "use opencl solver" OFF)

## Add solvers here
add_definitions(-DUSE_CPU_TROMP)
message("-- USE_CPU_TROMP DEFINED")

add_subdirectory(cpu_tromp)

if(USE_OCL_SILENTARMY)
add_subdirectory(ocl_silentarmy)
endif()

add_library(libminer STATIC MinerFactory.cpp MinerFactory.h speed.cpp libstratum/ZcashStratum.cpp libstratum/StratumClient.cpp)

target_compile_definitions(libminer PUBLIC -DUSE_CPU_TROMP)

target_link_libraries(libminer cpu_tromp)

if(USE_OCL_SILENTARMY)
target_compile_definitions(libminer PUBLIC -DUSE_OCL_SILENTARMY)
message("-- USE_OCL_SILENTARMY DEFINED")
target_link_libraries(libminer ocl_silentarmy)
endif()

target_include_directories(libminer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libminer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} .. ../..)
