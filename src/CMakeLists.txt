cmake_minimum_required(VERSION 3.10) 

set(SRC "./")
set(INCLUDE "./include" ${CMAKE_SOURCE_DIR}/src/)
set(SERIALIZER "./include/serializer")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${INCLUDE})
include_directories(${SERIALIZER})

file(GLOB FILESYSTEM_SRC ${SRC}filesystem/*.cpp ${SRC}filesystem/*.hpp)
file(GLOB RUNTIME_SRC ${SRC}runtime/*.cpp ${SRC}runtime/*.hpp)
file(GLOB EVENT_SRC ${SRC}event/*.cpp)
file(GLOB CORE_SRC ${SRC}core/error.cpp)
file(GLOB SHARED_MEM_SRC ${SRC}shared_memory/*.cpp ${SRC}shared_memory/*.h)
link_directories(${CMAKE_SOURCE_DIR}/lib)

set(SRC_FILES ${SHARED_MEM_SRC} ${CORE_SRC} ${FILESYSTEM_SRC} ${SRC}/filesystem/serialize_fs.cpp ${RUNTIME_SRC})

if(WIN32)
	add_executable(guthi_exec ${SRC_FILES} ${SRC}/main.cpp )
	add_library(GuthiCore STATIC ${SRC_FILES} ${SRC}/core/c_api.cpp )

	target_link_libraries(guthi_exec gdi32 kernel32 user32 pdh)
	target_link_libraries(GuthiCore gdi32 kernel32 user32 pdh)
	
	target_compile_features(guthi_exec PUBLIC cxx_std_20)

	if(MSVC)
		target_compile_options(guthi_exec PUBLIC)
	endif(MSVC)
endif(WIN32)

if(UNIX)
	# add_executable(guthi_exec ${SRC}/main.cpp ${FILESYSTEM_SRC} ${CORE_SRC} ${EVENT_SRC} ${SRC}/filesystem/cache.cpp  ${SRC}/filesystem/serialize_fs.cpp ${RUNTIME_SRC})
	target_link_libraries(guthi_exec pthread dl m)
	add_executable(guthi_exec ${SRC_FILES} ${SRC}/main.cpp)
add_library(GuthiCore STATIC ${SRC_FILES} ${SRC}/core/c_api.cpp)

	# set(CMAKE_CXX_FLAGS "-fsanitize=address")
endif(UNIX)
