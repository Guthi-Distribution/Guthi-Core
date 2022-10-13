cmake_minimum_required(VERSION 3.10) 

set(SRC "./")
set(INCLUDE "./include" ${CMAKE_SOURCE_DIR}/src/)
set(SERIALIZER "./include/serializer")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${INCLUDE})
include_directories(${SERIALIZER})

file(GLOB FILESYSTEM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem/*.cpp)
file(GLOB EVENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/event/*.cpp)
file(GLOB SHARED_MEM_SRC ${SRC}shared_memory/*.cpp ${SRC}shared_memory/*.h)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_SOURCE_DIR}/binary)

if(WIN32)
	add_executable(guthi_exec ${SRC}/main.cpp ${SHARED_MEM_SRC} ${SRC}/filesystem/fs.cpp ${SRC}/filesystem/cache.cpp ${SRC}/filesystem/serialize_fs.cpp ${SRC}/runtime/sys_info.cpp)
	target_link_libraries(guthi_exec gdi32 kernel32 user32)
	target_compile_features(guthi_exec PUBLIC cxx_std_20)

	if(MSVC)
		target_compile_options(guthi_exec PUBLIC)
	endif(MSVC)
endif(WIN32)

if(UNIX)
	add_executable(guthi_exec ${SRC}/main.cpp ${FILESYSTEM_SRC} ${EVENT_SRC} ${SRC}/filesystem/cache.cpp  ${SRC}/filesystem/serialize_fs.cpp ${SRC}/runtime/sys_info.cpp)
	target_link_libraries(guthi_exec pthread dl m)
	set(CMAKE_CXX_FLAGS "-fsanitize=address")
endif(UNIX)
