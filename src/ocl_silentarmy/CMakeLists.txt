set(EXECUTABLE ocl_silentarmy)

# depending on gcc version
# ;-std=c++11 => Ubuntu 14.04 check gcc versions
#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-std=c++11)

file(GLOB SRC_LIST
    ocl_silentarmy.cpp  sa_blake.cpp
)

file(GLOB HEADERS
    param.h  ocl_silentarmy.hpp
)

find_package(OpenCL REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${OpenCL_INCLUDE_DIRS})
include_directories(..)
ADD_LIBRARY(${EXECUTABLE} STATIC ${SRC_LIST} ${HEADERS})
TARGET_LINK_LIBRARIES(${EXECUTABLE} ${OpenCL_LIBRARIES} concore_crypto sodium)

target_compile_definitions(${EXECUTABLE} PUBLIC -DUSE_OCL_SILENTARMY)
