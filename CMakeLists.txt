cmake_minimum_required (VERSION 3.9)
project(MultiviewRendering)

## User-settable variables
set(RENDER_TOOL_DIR "3rdparty/Tools" CACHE PATH "root to rendering tools")

set(INCLUDE_DIRS)
set(LIBRARY_DIRS)
set(LIBRARIES)

macro(declare_include_path include_path)
	list(APPEND INCLUDE_DIRS ${include_path})
endmacro()

macro(declare_link_path link_path)
	list(APPEND LIBRARY_DIRS ${link_path})
endmacro()

macro(declare_lib lib)
	list(APPEND LIBRARIES "${lib}.lib")
endmacro()


## include
declare_include_path("3rdparty/glew/include")
declare_include_path("3rdparty/glm")
declare_include_path("3rdparty/glfw/include")
declare_include_path("3rdparty/Tools")

## link path
declare_link_path("3rdparty/glew/lib")
declare_link_path("3rdparty/glfw/lib")
declare_link_path("3rdparty/jpegturbo")
declare_link_path("3rdparty/webp")

## libs
declare_lib("glew32s")
declare_lib("glfw3")
declare_lib("opengl32")
declare_lib("turbojpeg")
declare_lib("libwebp")

# the search paths
include_directories(${INCLUDE_DIRS})
link_directories(${LIBRARY_DIRS})

# preprocessor
add_definitions(-DENABLE_TIMING=1)
add_definitions(-DENABLE_LOG=1)

if(MSVC)
add_definitions(-D_CRT_SECURE_NO_WARNING)
endif()

add_executable(
	MultiviewRendering 
	${RENDER_TOOL_DIR}/camera/Camera.cpp
	${RENDER_TOOL_DIR}/common/CommonIO.cpp
	${RENDER_TOOL_DIR}/image/ImageIO.cpp
	${RENDER_TOOL_DIR}/image/Image.cpp
	${RENDER_TOOL_DIR}/image/ImageCodec.cpp
	${RENDER_TOOL_DIR}/image/WebpCodec.cpp
	${RENDER_TOOL_DIR}/image/JpegCodec.cpp
	${RENDER_TOOL_DIR}/mesh/Geometry.cpp
	${RENDER_TOOL_DIR}/mesh/tiny_obj_loader.cc
	source/LFEngine.cpp
	source/LFLoader.cpp
	source/OBJRender.cpp
	source/UserInterface.cpp
	source/main.cpp
	)

target_link_libraries(MultiviewRendering ${LIBRARIES})

SUBDIRS(tools)