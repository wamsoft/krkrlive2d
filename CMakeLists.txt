cmake_minimum_required(VERSION 3.16)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_compile_options("$<$<AND:$<C_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:C>>:/utf-8>")
    add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/utf-8>")
    add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/Zc:__cplusplus>")
    project(dummy)
endif()

set(TPSTUB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../tp_stub CACHE PATH "Path to tp_stub directory")
include(${TPSTUB_DIR}/krkrz.cmake)

# live2d ドライバライブラリ
set(LIVE2DLIB_FOLDER d:/work/live2d/live2d)
option(USE_D3D9 "" OFF)
option(USE_SHARED "" OFF)
option(USE_GLES2 "" ON)
# live2d ドライバ側は SDK 配布用の install() 一式 (gles2glad/<arch>/lib/*.lib,
# .../include/*.h) を持つが、krkrz プラグインとして組み込むときの配布物は
# プラグイン DLL だけ。そのまま install すると bin/ に SDK 一式が混ざるので抑止する。
set(LIVE2DDRIVER_INSTALL OFF CACHE BOOL "Generate install rules for live2d driver SDK" FORCE)
add_subdirectory(${LIVE2DLIB_FOLDER} ${CMAKE_CURRENT_BINARY_DIR}/live2ddriver)
unset(CMAKE_SKIP_INSTALL_RULES)

krkrz_plugin(krkrlive2d
    NCBIND
    SOURCES
        src/Live2D.h
        src/Live2D.cpp
        src/utils.cpp
        src/MemoryAllocator.cpp
    LIBRARIES
        live2ddriver
)
