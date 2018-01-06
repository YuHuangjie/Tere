# - Try to find webp
# Once done this will define
#  WEBP_FOUND - System has webp
#  WEBP_INCLUDE_DIRS - The webp include directories
#  WEBP_LIBRARIES - The libraries needed to use webp
#  WEBP_DEFINITIONS - Compiler switches required for using webp
# reference: https://cmake.org/Wiki/CMake:How_To_Find_Libraries

if (APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(WEBP QUIET libwebp)
	set(WEBP_DEFINITIONS ${WEBP_CFLAGS_OTHER})

	find_path(WEBP_INCLUDE_DIR NAMES webp/decoder.h webp/types.h
		HINTS ${WEBP_INCLUDEDIR} ${WEBP_INCLUDE_DIRS}
		)

	find_library(WEBP_LIBRARY libwebpdecoder.dylib
		HINTS ${WEBP_LIBDIR} ${WEBP_LIBRARY_DIRS}
		)

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set JPEG_TURBO_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(webp  DEFAULT_MSG
	                                  WEBP_LIBRARY WEBP_INCLUDE_DIR)

	mark_as_advanced(WEBP_INCLUDE_DIR WEBP_LIBRARY )

	set(WEBP_LIBRARIES ${WEBP_LIBRARY})
	set(WEBP_INCLUDE_DIRS ${WEBP_INCLUDE_DIR})
elseif (WIN32 AND EXISTS "${DEPENDS_DIR}/webp")
	set(WEBP_FOUND TRUE)
	set(WEBP_INCLUDE_DIRS "${DEPENDS_DIR}/webp/include")
	set(WEBP_LIBRARIES "${DEPENDS_DIR}/webp/lib/win64/libwebp.lib")
endif()