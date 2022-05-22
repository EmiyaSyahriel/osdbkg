cmake_minimum_required(VERSION 3.8)
project(OSDBKG)
add_subdirectory("thirdparty/glm")

set(APP_NAME "osdbkg")
execute_process(
	COMMAND dotnet script "${CMAKE_CURRENT_SOURCE_DIR}/buildtools/GENRES.CSX"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
file(GLOB_RECURSE main_src "src/*.CPP" "src/*.HPP")

if(ANDROID)
	file(GLOB_RECURSE platform_src "platform/android/*.CPP" "platform/android/*.HPP")
	add_library(${APP_NAME} SHARED ${main_src} ${platform_src})
	target_link_libraries(${APP_NAME} PUBLIC GLESv3 EGL log m glm::glm z)
else()
	if(UNIX)
	set(OpenGL_GL_PREFERENCE "GLVND")
	endif()
	find_package(GLEW REQUIRED)
	find_package(glfw3 REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(ZLIB REQUIRED)
	file(GLOB_RECURSE platform_src "platform/desktop/*.CPP" "platform/desktop/*.HPP")
	add_executable(${APP_NAME} ${main_src} ${platform_src})
	target_link_libraries(${APP_NAME} PUBLIC GLEW::GLEW glfw OpenGL::GL glm::glm ZLIB::ZLIB)
endif()
