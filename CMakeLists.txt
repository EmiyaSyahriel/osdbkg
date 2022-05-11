project(OSDBKG)
add_subdirectory("thirdparty/glm")

set(APP_NAME "osdbkg")

file(GLOB_RECURSE main_src "src/*.cpp" "src/*.hpp")

if(ANDROID)
	file(GLOB_RECURSE desktop_src "platform/desktop/*.cpp" "platform/desktop/*.hpp")
	add_library(${APP_NAME} SHARED ${main_src} ${desktop_src})
	target_link_libraries()
elseif(WIN32 or UNIX or APPLE)
	find_package(glew REQUIRED)
	find_package(glfw3 REQUIRED)
	find_package(OpenGL REQUIRED)
	file(GLOB_RECURSE desktop_src "platform/desktop/*.cpp" "platform/desktop/*.hpp")
	add_executable(${APP_NAME} ${main_src} ${desktop_src})
endif()