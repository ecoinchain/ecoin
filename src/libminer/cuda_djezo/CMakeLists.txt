set(EXECUTABLE cuda_djezo)
set(AUTOMOC OFF)
set(AUTOUIC OFF)
set(AUTORCC OFF)

cmake_policy(SET CMP0058 OLD)

option(ENABLE_CUDA "Enable the cuda build" ON)

# depending on gcc version
# ;-std=c++11 => Ubuntu 14.04 check gcc versions
#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-std=c++11)

file(GLOB SRC_LIST
    cuda_djezo.cpp
    equi_miner.cu )
file(GLOB HEADERS
    cuda_djezo.hpp
    eqcuda.hpp
    )


#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-D_FORCE_INLINES;--disable-warnings;--ptxas-options=-v;-Xptxas=-dlcm=ca;-Xptxas=-dscm=cs; -O3)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};--disable-warnings;--ptxas-options=-v;-use_fast_math;-lineinfo; -O3)

set(CUDA_NVCC_COMPILE_DEFINITIONS [==[@CUDA_NVCC_COMPILE_DEFINITIONS@]==])

FIND_PACKAGE(CUDA REQUIRED)
if(COMPUTE AND (COMPUTE GREATER 0))
        LIST(APPEND CUDA_NVCC_FLAGS -gencode arch=compute_${COMPUTE},code=sm_${COMPUTE})
else(COMPUTE AND (COMPUTE GREATER 0))
        set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_50,code=sm_50; -gencode arch=compute_52,code=sm_52; -gencode arch=compute_60,code=sm_60; -gencode arch=compute_61,code=sm_61;)
endif(COMPUTE AND (COMPUTE GREATER 0))

if(CUDA_FOUND)
message("CUDA FOUND")
else()
message("CUDA NOT FOUND")
endif()


include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(..)
CUDA_ADD_LIBRARY(${EXECUTABLE} STATIC ${SRC_LIST} ${HEADERS})
TARGET_LINK_LIBRARIES(${EXECUTABLE} ${CUDA_LIBRARIES} cuda sodium concore_crypto blake2)

message("-- CUDA_NVCC_FLAGS: ${CUDA_NVCC_FLAGS}")

target_compile_definitions(${EXECUTABLE} PUBLIC -DUSE_CUDA_DJEZO)
