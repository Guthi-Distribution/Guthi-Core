cmake_minimum_required(VERSION 3.10)

set (SRC "./src")
set (INCLUDE "./include")

include_directories(${INCLUDE})

if(WIN32)	
	add_executable(guthi_exec ${SRC}/main.cpp ${SRC}/filesystem/fs.cpp ${SRC}/filesystem/cache.cpp)
	target_link_libraries(guthi_exec gdi32 kernel32 user32)
	target_compile_features(guthi_exec PUBLIC cxx_std_20)
	if(MSVC)
	target_compile_options(guthi_exec PUBLIC "/std:c++20")
	endif(MSVC)
endif (WIN32)

if (UNIX)
    add_executable(file_sharing ${SRC}/main.cpp)
	target_link_libraries(file_sharing pthread dl  m)
	set (CMAKE_CXX_FLAGS "-std=c++20 -fsanitize=address")
endif (UNIX)
