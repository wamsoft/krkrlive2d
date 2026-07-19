cmake_minimum_required(VERSION 3.16)

# UTF-8 source
add_compile_options("$<$<AND:$<C_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:C>>:/utf-8>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/utf-8>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/Zc:__cplusplus>")

if(WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
set(WIN64 1)
endif()

set(PROJECT_NAME krkrlive2d)
set(PROJECT_VERSION 1.00)

set(LIVE2DLIB_FOLDER d:/work/live2d/live2d)

include(../gles/vcpkg.cmake)

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})

option(USE_D3D9 "" OFF)
option(USE_SHARED "" OFF)
option(USE_GLES2 "" ON)
add_subdirectory(${LIVE2DLIB_FOLDER} ${CMAKE_CURRENT_BINARY_DIR}/live2ddriver)

add_subdirectory(../ncbind ${CMAKE_CURRENT_BINARY_DIR}/ncbind)

add_library(${PROJECT_NAME} SHARED
    src/Live2D.h
    src/Live2D.cpp
    src/utils.cpp
    src/MemoryAllocator.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    live2ddriver
    ncbind
)

if (WIN64)
set(PLUGIN_DIR "plugin64")
else()
set(PLUGIN_DIR "plugin")
endif()

# Custom install component for selective installation
install(TARGETS krkrlive2d
    RUNTIME DESTINATION ${PLUGIN_DIR}
    COMPONENT krkrplugins
    EXCLUDE_FROM_ALL
)
