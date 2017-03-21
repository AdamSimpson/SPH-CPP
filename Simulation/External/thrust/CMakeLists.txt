cmake_minimum_required(VERSION 2.8.8)
project(thrust_builder CXX)
include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    thrust
    PREFIX ${CMAKE_BINARY_DIR}/thrust
    GIT_REPOSITORY https://github.com/thrust/thrust.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
   )

ExternalProject_Get_Property(thrust source_dir)
set(THRUST_INCLUDE_DIR ${source_dir} CACHE INTERNAL "Path to include folder for Thrust")
