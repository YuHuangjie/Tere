# - Try to find glfw3
# Once done this will define
#  GLFW3_FOUND - System has glfw3
#  GLFW3_INCLUDE_DIRS - The glfw3 include directories
#  GLFW3_LIBRARIES - The libraries needed to use glfw3
#  GLFW3_DEFINITIONS - Compiler switches required for using glfw3
# reference: https://cmake.org/Wiki/CMake:How_To_Find_Libraries

if (APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GLFW3 QUIET glfw3)
	set(GLFW3_DEFINITIONS ${GLFW3_CFLAGS_OTHER})

	find_path(GLFW3_INCLUDE_DIR NAMES GLFW/glfw3.h
		HINTS ${GLFW3_INCLUDEDIR} ${GLFW3_INCLUDE_DIRS}
		)

	find_library(GLFW3_LIBRARY libglfw.dylib
		HINTS ${GLFW3_LIBDIR} ${GLFW3_LIBRARY_DIRS}
		)

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set JPEG_TURBO_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(glfw3  DEFAULT_MSG
	                                  GLFW3_LIBRARY GLFW3_INCLUDE_DIR)

	mark_as_advanced(GLFW3_INCLUDE_DIR GLFW3_LIBRARY )

	set(GLFW3_LIBRARIES ${GLFW3_LIBRARY})
	set(GLFW3_INCLUDE_DIRS ${GLFW3_INCLUDE_DIR})
elseif (WIN32 AND EXISTS "${DEPENDS_DIR}/glfw")
	set(GLFW3_FOUND TRUE)
	set(GLFW3_INCLUDE_DIRS "${DEPENDS_DIR}/glfw/include")
	set(GLFW3_LIBRARIES "${DEPENDS_DIR}/glfw/lib/glfw3.lib")
endif()