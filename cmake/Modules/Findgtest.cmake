# - Try to find gtest
# Once done this will define
#  GTEST_FOUND - System has gtest
#  GTEST_INCLUDE_DIRS - The gtest include directories
#  GTEST_LIBRARIES - The libraries needed to use gtest
#  GTEST_DEFINITIONS - Compiler switches required for using gtest
# reference: https://cmake.org/Wiki/CMake:How_To_Find_Libraries

if (APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTEST QUIET libgtest)
	set(GTEST_DEFINITIONS ${GTEST_CFLAGS_OTHER})

	find_path(GTEST_INCLUDE_DIR NAMES gtest/decoder.h gtest/types.h
		HINTS ${GTEST_INCLUDEDIR} ${GTEST_INCLUDE_DIRS}
		)

	find_library(GTEST_LIBRARY libgtestdecoder.dylib
		HINTS ${GTEST_LIBDIR} ${GTEST_LIBRARY_DIRS}
		)

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set JPEG_TURBO_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(gtest  DEFAULT_MSG
	                                  GTEST_LIBRARY GTEST_INCLUDE_DIR)

	mark_as_advanced(GTEST_INCLUDE_DIR GTEST_LIBRARY )

	set(GTEST_LIBRARIES ${GTEST_LIBRARY})
	set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})
elseif (WIN32 AND EXISTS "${DEPENDS_DIR}/gtest")
	set(GTEST_FOUND TRUE)
	set(GTEST_INCLUDE_DIRS "${DEPENDS_DIR}/gtest/include")
	set(GTEST_LIBRARIES "${DEPENDS_DIR}/gtest/lib/gtestd.lib" "${DEPENDS_DIR}/gtest/lib/gtest_maind.lib")
endif()