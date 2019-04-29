###############################################################################
# TereSample
###############################################################################

project(TereSample)

# Specifiy necessary dependencies
if (WIN32)
	# Use vcpkg to resolve dependencies on Windows platform
	set(CMAKE_TOOLCHAIN_FILE 
		"C:/vcpkg/scripts/buildsystems/vcpkg.cmake" 
		CACHE 
		FILEPATH 
		"Path of vcpkg tool chain file"
	)
endif()

# Require opengl
find_package(OpenGL REQUIRED)

find_package(OpenMP)

# Require glfw
find_package(glfw3 CONFIG REQUIRED)

# Require glew
find_package(JPEG REQUIRED)
SET(TURBOJPEG_LIBRARIES 
	optimized 
	turbojpeg 
	debug 
	turbojpegd
	)

include_directories("${CMAKE_SOURCE_DIR}/TereMain/include")

add_executable(
	TereSample 
	main.cpp
	mesh/tiny_obj_loader.cc
	mesh/tinyply.cpp
	mesh/Geometry.cpp
	image/ImageCodec.cpp
	image/JpegCodec.cpp
	ProfileIO.cpp
	)

if (WIN32 OR UNIX)
	add_executable(
		TereSample 
		glew.c
		)
	add_definitions(-DGLEW_STATIC)
endif()

target_link_libraries(TereSample 
	PRIVATE
	${OpenGL_LIBRARIES}
	glfw
	${TURBOJPEG_LIBRARIES}
	Tere
	)
if(OpenMP_CXX_FOUND)
    target_link_libraries(
		TereSample 
		PRIVATE 
		OpenMP::OpenMP_CXX
		)
endif()
target_include_directories(
	TereSample 
	PRIVATE
	)

# installation
install(TARGETS 
	TereSample 
	RUNTIME 
	DESTINATION 
	"bin"
	)
