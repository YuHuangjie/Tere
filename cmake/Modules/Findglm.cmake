# - Try to find glm
# Once done this will define
#  GLM_FOUND - System has glm
#  GLM_INCLUDE_DIRS - The glm include directories
#  GLM_LIBRARIES - The libraries needed to use glm
#  GLM_DEFINITIONS - Compiler switches required for using glm
# reference: https://cmake.org/Wiki/CMake:How_To_Find_Libraries

if (APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GLM QUIET glm)
	set(GLM_DEFINITIONS ${GLM_CFLAGS_OTHER})

	find_path(GLM_INCLUDE_DIR NAMES glm/glm.hpp
		HINTS ${GLM_INCLUDEDIR} ${GLM_INCLUDE_DIRS}
		)

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set JPEG_TURBO_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(glm  DEFAULT_MSG
	                                  GLM_INCLUDE_DIR)

	mark_as_advanced(GLM_INCLUDE_DIR )

	set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
elseif(WIN32 AND EXISTS "${DEPENDS_DIR}/glm")
	set(GLM_FOUND TRUE)
	set(GLM_INCLUDE_DIRS "${DEPENDS_DIR}/glm")
endif()