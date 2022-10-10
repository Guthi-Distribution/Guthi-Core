cmake_minimum_required(VERSION 3.10) 

set(SRC "./")
set(INCLUDE "./include" ${CMAKE_SOURCE_DIR}/src/)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${INCLUDE})

file(GLOB FILESYSTEM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem/*.cpp)
file(GLOB EVENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/event/*.cpp)
file(GLOB EVENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/shared_memory/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/shared_memory/*.h)
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
	set(CMAKE_CXX_FLAGS "-fsanitize=address")
	add_link_options("-fsanitize=address")
	add_executable(file_sharing ${SRC}/main.cpp ${FILESYSTEM_SRC} ${EVENT_SRC})
	target_link_libraries(file_sharing pthread dl m)
endif(UNIX)
