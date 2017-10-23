cmake_minimum_required (VERSION 3.9)
project(LumigraphRendering)

## User-settable variables
set(GLEW_DIR "D:/Program\ Files/glew-2.0.0" CACHE PATH "root to glew")
set(GLFW_DIR "D:/Program\ Files/glfw-3.2.1-64" CACHE PATH "root to glfw")
set(GLM_DIR "D:/Program\ Files/glm" CACHE PATH "root to glm")
set(JPEG_TURBO_DIR "D:/Program\ Files/libjpeg-turbo64" CACHE PATH "root to jpeg turbo")
set(WEBP_DIR "D:/Program\ Files/libwebp-0.6.0-windows-x64" CACHE PATH "root to webp library")

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
declare_include_path("${GLEW_DIR}/include")
declare_include_path("${GLM_DIR}")
declare_include_path("${GLFW_DIR}/include")

## link path
declare_link_path("${GLEW_DIR}/lib/Release/x64")
declare_link_path("${GLFW_DIR}/lib-vc2015")
declare_link_path("${JPEG_TURBO_DIR}/lib")
declare_link_path("${WEBP_DIR}/lib")

## libs
declare_lib("glew32s")
declare_lib("glfw3")
declare_lib("opengl32")
declare_lib("turbojpeg")
declare_lib("libwebp")

# the search paths
include_directories(${INCLUDE_DIRS})
link_directories(${LIBRARY_DIRS})

add_executable(
	LumigraphRendering 
	source/ClosestCamera.cpp
	source/DecodeJpeg.cpp
	source/Decoder.cpp
	source/DecodeWebp.cpp
	source/LFEngine.cpp
	source/LFLoader.cpp
	source/OBJRender.cpp
	source/RenderCamView.cpp
	source/tinyobj.cpp
	source/UserInterface.cpp
	source/main.cpp
	)

target_link_libraries(LumigraphRendering ${LIBRARIES})