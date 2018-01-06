###############################################################################
# TereMain core library
###############################################################################

project(TereMain)

set(CMAKE_CXX_STANDARD 11)

# Require OpenGL
find_package(OpenGL REQUIRED)

# Require glm
find_package(GLM REQUIRED)
if (NOT GLM_FOUND)
	message(SEND_ERROR "Package glm required, but not found!")
endif()

# Require glew
find_package(GLEW REQUIRED)
if (NOT GLEW_FOUND)
	message(SEND_ERROR "Package glew required, but not found!")
endif()

# Require jpeg-turbo
find_package(jpeg-turbo REQUIRED)
if (NOT JPEG-TURBO_FOUND)
	message(SEND_ERROR "Package jpeg-turbo required, but not found!")
endif()

# Require webp
find_package(webp REQUIRED)
if (NOT WEBP_FOUND)
	message(SEND_ERROR "Package webp required, but not found")
endif()


# define header directories and source files for the library
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Add dependent headers
include_directories("${GLM_INCLUDE_DIRS}")
include_directories("${GLEW_INCLUDE_DIRS}")
include_directories("${JPEG-TURBO_INCLUDE_DIRS}")
include_directories("${WEBP_INCLUDE_DIRS}")

# Add needed definitions
add_definitions(-DENABLE_TIMING=1)
add_definitions(-DENABLE_LOG=1)
add_definitions(-DGLEW_STATIC)

if (BUILD_STATIC_LIBRARY)
	set(TERE_LIB_TYPE STATIC)
else()
	set(TERE_LIB_TYPE SHARED)
endif()

add_library(TereMain ${TERE_LIB_TYPE} ${SOURCE_FILES})
target_link_libraries(TereMain 
	${GLEW_LIBRARIES} 
	${JPEG-TURBO_LIBRARIES}
	${WEBP_LIBRARIES}
	${OPENGL_LIBRARY}
)

# Visual Studio 2015 complains "error LNK2019" on jpegturbo-static.lib
if (WIN32)
	target_link_libraries(TereMain "legacy_stdio_definitions.lib")
endif()

# install TereMain
install(TARGETS TereMain
	RUNTIME DESTINATION "bin"
	ARCHIVE DESTINATION "lib"
	LIBRARY DESTINATION "lib")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
	DESTINATION include/Tere)

# install dependencies
install(DIRECTORY "${GLEW_INCLUDE_DIRS}/"
	DESTINATION include/glew)
install(DIRECTORY "${GLM_INCLUDE_DIRS}/"
	DESTINATION include/glm)

if (WIN32)
	install(PROGRAMS "${GLEW_LIBRARIES}"
		DESTINATION lib)
	install(PROGRAMS "${JPEG-TURBO_LIBRARIES}"
		DESTINATION lib)
	install(PROGRAMS "${JPEG-TURBO_RUNTIME}"
		DESTINATION bin)
	install(PROGRAMS "${WEBP_LIBRARIES}"
		DESTINATION lib)
endif()