# - Try to find jpeg-turbo
# Once done this will define
#  JPEG_TURBO_FOUND - System has jpeg-turbo
#  JPEG_TURBO_INCLUDE_DIRS - The jpeg-turbo include directories
#  JPEG_TURBO_LIBRARIES - The libraries needed to use jpeg-turbo
#  JPEG_TURBO_DEFINITIONS - Compiler switches required for using jpeg-turbo
# reference: https://cmake.org/Wiki/CMake:How_To_Find_Libraries

if (APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(JPEG-TURBO QUIET libturbojpeg)
	set(JPEG-TURBO_DEFINITIONS ${JPEG-TURBO_CFLAGS_OTHER})

	find_path(JPEG-TURBO_INCLUDE_DIR turbojpeg.h
		HINTS ${JPEG-TURBO_INCLUDEDIR} ${JPEG-TURBO_INCLUDE_DIRS}
		)

	find_library(JPEG-TURBO_LIBRARY libturbojpeg.dylib
		HINTS ${JPEG-TURBO_LIBDIR} ${JPEG-TURBO_LIBRARY_DIRS}
		)

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set JPEG_TURBO_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(jpeg-turbo  DEFAULT_MSG
	                                  JPEG-TURBO_LIBRARY JPEG-TURBO_INCLUDE_DIR)

	mark_as_advanced(JPEG-TURBO_INCLUDE_DIR JPEG-TURBO_LIBRARY )

	set(JPEG-TURBO_LIBRARIES ${JPEG-TURBO_LIBRARY})
	set(JPEG-TURBO_INCLUDE_DIRS ${JPEG-TURBO_INCLUDE_DIR})
elseif (WIN32 AND EXISTS "${DEPENDS_DIR}/jpegturbo")
	set(JPEG-TURBO_FOUND TRUE)
	set(JPEG-TURBO_INCLUDE_DIRS "${DEPENDS_DIR}/jpegturbo1.3/include")
	set(JPEG-TURBO_LIBRARIES "${DEPENDS_DIR}/jpegturbo1.3/lib/win64/turbojpeg.lib")
	set(JPEG-TURBO_RUNTIME "${DEPENDS_DIR}/jpegturbo1.3/lib/win64/turbojpeg.dll")
endif()