###############################################################################
# TereSample
###############################################################################

project(TereSample)

if (BUILD_SAMPLES)
	# Require opengl
	find_package(OPENGL REQUIRED)
	if (NOT OPENGL_FOUND)
		message(SEND_ERROR "Package OpenGL required, but not found!")
	endif()

	# Require glfw
	find_package(GLFW3 REQUIRED)
	if (NOT GLFW3_FOUND)
		message(SEND_ERROR "Package glfw3 required, but not found!")
	endif()

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

	# Require glew
	find_package(WEBP REQUIRED)
	if (NOT WEBP_FOUND)
		message(SEND_ERROR "Package webp required, but not found!")
	endif()

	# Require glew
	find_package(JPEG-TURBO REQUIRED)
	if (NOT JPEG-TURBO_FOUND)
		message(SEND_ERROR "Package JPEG-TURBO required, but not found!")
	endif()

	include_directories("${GLFW3_INCLUDE_DIRS}")
	include_directories("${GLEW_INCLUDE_DIRS}")
	include_directories("${GLM_INCLUDE_DIRS}")
	include_directories("${CMAKE_SOURCE_DIR}/TereMain/include")

	link_directories("${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

	add_executable(
		TereSample 
		main.cpp
		)
	add_dependencies(TereSample TereMain)

	target_link_libraries(TereSample 
		${OPENGL_LIBRARY}
		${GLFW3_LIBRARIES} 
		${GLEW_LIBRARIES} 
		${WEBP_LIBRARIES} 
		${JPEG-TURBO_LIBRARIES}
		TereMain
		)

	# installation
	install(TARGETS TereSample 
		RUNTIME DESTINATION "bin")
endif()