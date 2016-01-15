# - Try to find GLM headers
# Once done this will define
#  GLM_FOUND - System has GLM
#  GLM_INCLUDE_DIRS - The GLM include directories
#  GLM_DEFINITIONS - Compiler switches required for using GLM

find_path(GLM_INCLUDE_DIR glm/glm.hpp)

set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set GLM to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GLM  DEFAULT_MSG
                                  GLM_INCLUDE_DIRS)

mark_as_advanced(GLM_INCLUDE_DIRS)
