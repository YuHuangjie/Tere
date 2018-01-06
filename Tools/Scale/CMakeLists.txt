###############################################################################
# RearrangeMesh (Tool)
###############################################################################

project(Scale)

# Require glm
if(WIN32 AND EXISTS "${DEPENDS_DIR}/glm")
	set(GLM_FOUND TRUE)
	set(GLM_INCLUDE_DIRS "${DEPENDS_DIR}/glm")
elseif (APPLE)
	find_package(GLM REQUIRED)
endif()

if (NOT GLM_FOUND)
	message(SEND_ERROR "Package glm required, but not found!")
endif()

include_directories(${GLM_INCLUDE_DIRS})
include_directories("${CMAKE_SOURCE_DIR}/TereMain/include")

add_executable(
	Scale 
	main.cpp
	ObjIO.cpp
	LFLoader.cpp
	)

# install Scale
install(TARGETS Scale 
	RUNTIME DESTINATION "bin")