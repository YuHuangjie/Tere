###############################################################################
# TERE BUILD SYSTEM
#     (Two point Eight Rendering Engine)
###############################################################################

cmake_minimum_required (VERSION 3.9)

## CUDA support ?
option(USE_CUDA "Use CUDA" OFF)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY 
	${CMAKE_BINARY_DIR}/bin 
	)

# Supress annoying warnings
if (WIN32)
	# Type conversions
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4267")
endif()

# Setup TereMain project
add_subdirectory(TereMain)

# Setup Tools
# add_subdirectory(Tools)

# Setup samples
if (WIN32 OR UNIX)
	set(BUILD_SAMPLES 
		FALSE 
		CACHE 
		BOOL 
		"Build samples"
		)
else()
	set(BUILD_SAMPLES 
		FALSE
		)
endif()

if (BUILD_SAMPLES)
	add_subdirectory(Samples)
endif()

# Setup tests
# set(BUILD_TESTS TRUE CACHE BOOL "Build tests")
# add_subdirectory(Test)

