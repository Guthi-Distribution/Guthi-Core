cmake_minimum_required(VERSION 3.10) 

project(Guthi) 
cmake_minimum_required(VERSION 3.10)

set(SRC "./src")
set(INCLUDE "./include")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${INCLUDE})
set(CAMKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

file(GLOB FILESYSTEM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem/*.cpp)
file(GLOB EVENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/event/*.cpp)
set(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_SOURCE_DIR}/binary)

if(WIN32)
	add_executable(guthi_exec ${SRC}/main.cpp ${SRC}/filesystem/fs.cpp ${SRC}/filesystem/cache.cpp)
	target_link_libraries(guthi_exec gdi32 kernel32 user32)
	target_compile_features(guthi_exec PUBLIC cxx_std_20)

	if(MSVC)
		target_compile_options(guthi_exec PUBLIC)
	endif(MSVC)
endif(WIN32)

if(UNIX)
	add_executable(file_sharing ${SRC}/main.cpp ${FILESYSTEM_SRC} ${EVENT_SRC})
	target_link_libraries(file_sharing pthread dl m)
	set(CMAKE_CXX_FLAGS "-fsanitize=address")
endif(UNIX)
