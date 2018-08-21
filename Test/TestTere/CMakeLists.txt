###############################################################################
# TereMain core library
###############################################################################

project(TestTere)

set(CMAKE_CXX_STANDARD 11)

# Require google test
find_package(gtest REQUIRED)

# Require glm
find_package(GLM REQUIRED)
if (NOT GLM_FOUND)
	message(SEND_ERROR "Package glm required, but not found!")
endif()

# define header directories and source files for the library
include_directories("${CMAKE_SOURCE_DIR}/TereMain/include")
include_directories("${GTEST_INCLUDE_DIRS}")
include_directories("${GLM_INCLUDE_DIRS}")

link_directories("${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_executable(
		TestTere
		TestCircleUI.cpp
		)
add_dependencies(TestTere TereMain)

target_link_libraries(
	TestTere
	${GTEST_LIBRARIES}
	TereMain.lib
	)