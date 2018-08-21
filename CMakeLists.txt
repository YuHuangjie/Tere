###############################################################################
# TERE BUILD SYSTEM
#     (Two point Eight Rendering Engine)
###############################################################################

cmake_minimum_required (VERSION 3.0.2)
project(TERE)

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  	# We don't want to install in default system location, install is really for the SDK, so call it that
  	set(CMAKE_INSTALL_PREFIX 
  	"${CMAKE_BINARY_DIR}/install" CACHE PATH "Directory to which binaries are to be installed" FORCE
  	)
endif()

# Specifiy necessary dependencies (glm, glew, jpegturbo, webp)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
if (WIN32)
	set(DEPENDS_DIR "" CACHE PATH "Directory where dependencies are installed")
endif(WIN32)

# Specify library type
set(BUILD_STATIC_LIBRARY TRUE)

# Build samples
set(BUILD_SAMPLES FALSE CACHE BOOL "")

# Setup TereMain project
add_subdirectory(TereMain)

# Setup Tools
add_subdirectory(Tools)

# Setup samples
add_subdirectory(Samples)

# Setup tests
set(BUILD_TESTS TRUE CACHE BOOL "Build tests")
add_subdirectory(Test)