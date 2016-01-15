cmake_minimum_required(VERSION 3.1)
project(sph-renderer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../Common/CMake)

set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED TRUE)

file(GLOB SRC_FILES src/*.cpp)
include_directories(".")

add_executable(sph-renderer ${SRC_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Common)

#set(MPI_CXX_COMPILER mpic++)
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
target_link_libraries(sph-renderer ${MPI_CXX_LIBRARIES})

set(Boost_NO_BOOST_CMAKE TRUE) # Ignore system boost and use module system boost
find_package(Boost COMPONENTS mpi system filesystem serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(sph-renderer ${Boost_LIBRARIES})

find_package(SFML 2 COMPONENTS system window graphics)
target_link_libraries(sph-renderer ${SFML_LIBRARIES})

find_package(GLM)
include_directories(${GLM_INCLUDE_DIRS})

find_package(OpenGL)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(${OPENGL_LIBRARIES})

install(TARGETS sph-renderer DESTINATION bin)
