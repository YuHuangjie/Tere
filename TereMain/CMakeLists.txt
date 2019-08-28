###############################################################################
# Tere core rendering library
###############################################################################

project(Tere)

set(CMAKE_CXX_STANDARD 11)

## Enable CUDA
if (USE_CUDA)
	enable_language(CUDA)
endif()

# Require OpenGL
find_package(OpenGL REQUIRED)

# define header directories and source files for the library
include_directories(include)

file(GLOB 
	SOURCE_FILES src/*.cpp
	)

# Add needed definitions
add_definitions(-DUSE_TIMING)
add_definitions(-DUSE_LOG)

# control maximum interpolation references
#add_definitions(-DMAX_NUM_INTERP=20)

if (USE_CUDA)
	add_definitions(-DUSE_CUDA)

	list(APPEND 
		SOURCE_FILES 
		src/BoundingBox.cu
		)
endif()

# Platform-dependent configuration
if (WIN32 OR UNIX)
	list(APPEND 
		SOURCE_FILES 
		src/glew.c
		)
	add_definitions(-DGLEW_STATIC)
endif()

# Unrecognized characters
if (USE_CUDA)
	set(CMAKE_CUDA_FLAGS "${CUDA_NVCC_FLAGS} -Xcompiler \"/wd 4819\"")
endif()

add_library(
	Tere SHARED 
	${SOURCE_FILES}
	)

target_link_libraries(
	Tere 
	${OPENGL_LIBRARY}
	)

if (USE_CUDA)
	target_link_libraries(
		Tere 
		cudart_static.lib
	)
endif()

# install libTere
install(TARGETS Tere
	RUNTIME DESTINATION "bin"
	ARCHIVE DESTINATION "lib"
	LIBRARY DESTINATION "lib"
	)

install(FILES 
	"include/LFEngine.h"
	"include/Type.h"
	DESTINATION 
	include/Tere
	)
