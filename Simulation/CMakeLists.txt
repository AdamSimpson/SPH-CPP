cmake_minimum_required(VERSION 3.1)
project(sph)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../Common/CMake/)
Message(${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 14) # Doesn't work with NVCC
set(CXX_STANDARD_REQUIRED TRUE)

# Set source files
file(GLOB SRC_FILES src/*.cpp)
file(GLOB TEST_SRC_FILES Tests/*.cpp)

# Add a bit more verbose output
add_definitions(-Wall)

# Find packages required for all builds
find_package(MPI REQUIRED)

set(Boost_NO_BOOST_CMAKE TRUE) # Ignore system boost and use module system boost
find_package(Boost COMPONENTS system filesystem REQUIRED)

find_package(Thrust REQUIRED)

set(ENV{ADIOS_DIR} "/usr")
message("Setting ADIOS_DIR manually to /usr")
find_package(ADIOS)

# Includes Catch in the project:
add_subdirectory(External/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

# Add includes
include_directories(".", "./src")
include_directories(sph, ${CMAKE_CURRENT_SOURCE_DIR}/../Common)
include_directories(${MPI_INCLUDE_PATH})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${THRUST_INCLUDE_DIRS})
include_directories(${ADIOS_INCLUDE_DIRS})
include_directories(${CATCH_INCLUDE_DIR})

# Setup for CUDA
if(${CUDA})
  Message("Enabling CUDA")
  find_package(CUDA REQUIRED)
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-DCUDA -ccbin /usr/bin/g++-5 --expt-extended-lambda --expt-relaxed-constexpr --std=c++11)
  set_source_files_properties( ${SRC_FILES} PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
  set_source_files_properties( ${TEST_SRC_FILES} PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
  cuda_add_executable(sph ${SRC_FILES})
  cuda_add_executable(sph_tests ${TEST_SRC_FILES})
endif()

# setup for OpenMP
if(${OPENMP})
  Message("Enabling OpenMP")
  find_package(OpenMP REQUIRED)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set_source_files_properties(${SRC_FILES} PROPERTIES LANGUAGE CXX)
  add_executable(sph ${SRC_FILES})
  add_executable(sph_tests ${TEST_SRC_FILES})
  add_definitions("-x c++ -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP")
  set_target_properties(sph PROPERTIES LINKER_LANGUAGE CXX)
endif()

# Setup for CUDA and OPENMP
if(${CUDA_AND_OPENMP})
  Message("Enabling CUDA")
  find_package(CUDA REQUIRED)
  find_package(OpenMP REQUIRED)
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-DCUDA -ccbin /usr/bin/g++-5 --expt-extended-lambda --expt-relaxed-constexpr --std=c++11 -Xcompiler ${OpenMP_CXX_FLAGS})
  set_source_files_properties( ${SRC_FILES} PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
  set_source_files_properties( ${TEST_SRC_FILES} PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
  cuda_add_executable(sph ${SRC_FILES})
  cuda_add_executable(sph_tests ${TEST_SRC_FILES})
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_dependencies(sph_tests catch)
add_test(NAME SphTests COMMAND sph_tests)

# Add link libraries
if(${CUDA_DEBUG})
  target_link_libraries(sph ${CUDA_TOOLKIT_ROOT_DIR}/lib64/libnvToolsExt.so)
  target_link_libraries(sph_tests ${CUDA_TOOLKIT_ROOT_DIR}/lib64/libnvToolsExt.so)
endif()

target_link_libraries(sph ${MPI_C_LIBRARIES})
target_link_libraries(sph ${Boost_LIBRARIES})
target_link_libraries(sph ${ADIOS_LIBRARIES})
target_link_libraries(sph_tests Catch)

# Install binaries
install(TARGETS sph DESTINATION bin)
install(TARGETS sph_tests DESTINATION bin)
