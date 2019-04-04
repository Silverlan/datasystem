cmake_minimum_required(VERSION 3.12)

set(PROJ_NAME datasystem)
project(${PROJ_NAME} CXX)

set(CMAKE_CXX_STANDARD 17)

set(DEPENDENCY_GLM_INCLUDE "" CACHE PATH "Path to GLM include directory.")
set(DEPENDENCY_SHAREDUTILS_INCLUDE "" CACHE PATH "Path to sharedutils include directory.")
set(DEPENDENCY_SHAREDUTILS_LIBRARY "" CACHE FILEPATH "Path to sharedutils library.")
set(DEPENDENCY_MATHUTIL_INCLUDE "" CACHE PATH "Path to mathutil include directory.")
set(DEPENDENCY_MATHUTIL_LIBRARY "" CACHE FILEPATH "Path to mathutil library.")
set(DEPENDENCY_VFILESYSTEM_INCLUDE "" CACHE PATH "Path to vfilesystem include directory.")
set(DEPENDENCY_VFILESYSTEM_LIBRARY "" CACHE FILEPATH "Path to vfilesystem library.")
set(DEPENDENCY_EXPRTK_INCLUDE "" CACHE PATH "Path to exprtk include directory.")

set(LIB_TYPE SHARED)

add_definitions(-DDATASYSTEM_DLL)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)

if(WIN32)
	add_compile_options(/bigobj)
endif()

function(def_vs_filters FILE_LIST)
	foreach(source IN LISTS FILE_LIST)
	    get_filename_component(source_path "${source}" PATH)
	    string(REPLACE "${CMAKE_CURRENT_LIST_DIR}" "" source_path_relative "${source_path}")
	    string(REPLACE "/" "\\" source_path_msvc "${source_path_relative}")
	    source_group("${source_path_msvc}" FILES "${source}")
	endforeach()
endfunction(def_vs_filters)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/*.hpp"
)
add_library(${PROJ_NAME} ${LIB_TYPE} ${SRC_FILES})
if(WIN32)
	target_compile_options(${PROJ_NAME} PRIVATE /wd4251)
	target_compile_options(${PROJ_NAME} PRIVATE /wd4996)
endif()
def_vs_filters("${SRC_FILES}")

target_link_libraries(${PROJ_NAME} ${DEPENDENCY_SHAREDUTILS_LIBRARY})
target_link_libraries(${PROJ_NAME} ${DEPENDENCY_MATHUTIL_LIBRARY})
target_link_libraries(${PROJ_NAME} ${DEPENDENCY_VFILESYSTEM_LIBRARY})

set_target_properties(${PROJ_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJ_NAME} PRIVATE ${DEPENDENCY_GLM_INCLUDE})
target_include_directories(${PROJ_NAME} PRIVATE ${DEPENDENCY_SHAREDUTILS_INCLUDE})
target_include_directories(${PROJ_NAME} PRIVATE ${DEPENDENCY_MATHUTIL_INCLUDE})
target_include_directories(${PROJ_NAME} PRIVATE ${DEPENDENCY_VFILESYSTEM_INCLUDE})
target_include_directories(${PROJ_NAME} PRIVATE ${DEPENDENCY_EXPRTK_INCLUDE})
