cmake_minimum_required(VERSION 3.1)
project(tests)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../Common/CMake/)
Message(${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED TRUE)

file(GLOB SRC_FILES *.cpp)
include_directories(".")

add_executable(tests ${SRC_FILES})

# For when compiling without NVCC
add_definitions(-DNO_CUDA -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CPP)
MESSAGE("DISABLING CUDA")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Common)

#set(MPI_CXX_COMPILER mpic++)
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
target_link_libraries(tests ${MPI_CXX_LIBRARIES})

set(Boost_NO_BOOST_CMAKE TRUE) # Ignore system boost and use module system boost
find_package(Boost COMPONENTS mpi system filesystem serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(tests ${Boost_LIBRARIES})

find_package(Thrust REQUIRED)
include_directories(${THRUST_INCLUDE_DIRS})

set(ENV{ADIOS_DIR} "/usr")
message("Setting ADIOS_DIR manually")
find_package(ADIOS)
include_directories(${ADIOS_INCLUDE_DIRS})
target_link_libraries(tests ${ADIOS_LIBRARIES})
