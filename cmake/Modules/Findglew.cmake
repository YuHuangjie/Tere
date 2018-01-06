# - Try to find glew
# Once done this will define
#  GLEW_FOUND - System has glew
#  GLEW_INCLUDE_DIRS - The glew include directories
#  GLEW_LIBRARIES - The libraries needed to use glew
#  GLEW_DEFINITIONS - Compiler switches required for using glew
# reference: https://cmake.org/Wiki/CMake:How_To_Find_Libraries

if (APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GLEW QUIET glew)
	set(GLEW_DEFINITIONS ${GLEW_CFLAGS_OTHER})

	find_path(GLEW_INCLUDE_DIR NAMES GL/glew.h
		HINTS ${GLEW_INCLUDEDIR} ${GLEW_INCLUDE_DIRS}
		)

	find_library(GLEW_LIBRARY libglew.dylib
		HINTS ${GLEW_LIBDIR} ${GLEW_LIBRARY_DIRS}
		)

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set JPEG_TURBO_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(glew  DEFAULT_MSG
	                                  GLEW_LIBRARY GLEW_INCLUDE_DIR)

	mark_as_advanced(GLEW_INCLUDE_DIR GLEW_LIBRARY )

	set(GLEW_LIBRARIES ${GLEW_LIBRARY})
	set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
elseif (WIN32 AND EXISTS "${DEPENDS_DIR}/glew")
	set(GLEW_FOUND TRUE)
	set(GLEW_INCLUDE_DIRS "${DEPENDS_DIR}/glew/include")
	set(GLEW_LIBRARIES "${DEPENDS_DIR}/glew/lib/glew32s.lib")
endif()