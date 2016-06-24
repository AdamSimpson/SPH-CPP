cmake_minimum_required(VERSION 3.1)
project(sph)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../Common/CMake/)
Message(${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 14) # Doesn't work with NVCC
set(CXX_STANDARD_REQUIRED TRUE)

file(GLOB SRC_FILES src/*.cu)

add_definitions(-Wall)

include_directories(".")
include_directories(sph, ${CMAKE_CURRENT_SOURCE_DIR}/../Common)

# Setup for CUDA
if(${CUDA})
  Message("Enabling CUDA")
  find_package(CUDA REQUIRED)
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-DCUDA -ccbin /usr/bin/g++-5 --expt-extended-lambda --expt-relaxed-constexpr --std=c++11)
  cuda_add_executable(sph ${SRC_FILES})
endif()

# setup for OpenMP
if(${OPENMP})
  Message("Enabling OpenMP")
  find_package(OpenMP REQUIRED)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set_source_files_properties(${SRC_FILES} PROPERTIES LANGUAGE CXX)
  add_executable(sph ${SRC_FILES})
  add_definitions("-x c++ -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP")
  set_target_properties(sph PROPERTIES LINKER_LANGUAGE CXX)
endif()

# Setup for CUDA and OPENMP
if(${CUDA_AND_OPENMP})
  Message("Enabling CUDA")
  find_package(CUDA REQUIRED)
  find_package(OpenMP REQUIRED)
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-DCUDA -ccbin /usr/bin/g++-5 --expt-extended-lambda --expt-relaxed-constexpr --std=c++11 -Xcompiler ${OpenMP_CXX_FLAGS})
  cuda_add_executable(sph ${SRC_FILES})
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(${CUDA_DEBUG})
target_link_libraries(sph ${CUDA_TOOLKIT_ROOT_DIR}/lib64/libnvToolsExt.so)
endif()

#set(MPI_CXX_COMPILER mpic++)
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
target_link_libraries(sph ${MPI_C_LIBRARIES})

set(Boost_NO_BOOST_CMAKE TRUE) # Ignore system boost and use module system boost
find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(sph ${Boost_LIBRARIES})

find_package(Thrust REQUIRED)
include_directories(${THRUST_INCLUDE_DIRS})

set(ENV{ADIOS_DIR} "/usr")
message("Setting ADIOS_DIR manually")
find_package(ADIOS)
include_directories(${ADIOS_INCLUDE_DIRS})
target_link_libraries(sph ${ADIOS_LIBRARIES})

install(TARGETS sph DESTINATION bin)
