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
#   Live2D Cubism Core は環境変数 CUBISM_SDK (ドライバ側 CMake が
#   $ENV{CUBISM_SDK}/Core を参照) から探索される。README.md 参照。
option(USE_D3D9 "" OFF)
option(USE_SHARED "" OFF)
option(USE_GLES2 "" ON)
# live2d ドライバ側は SDK 配布用の install() 一式 (gles2glad/<arch>/lib/*.lib,
# .../include/*.h) を持つが、krkrz プラグインとして組み込むときの配布物は
# プラグイン DLL だけ。そのまま install すると bin/ に SDK 一式が混ざるので抑止する。
set(LIVE2DDRIVER_INSTALL OFF CACHE BOOL "Generate install rules for live2d driver SDK" FORCE)

# ドライバソースの取得方針:
#   1. 環境変数 LIVE2DLIB_FOLDER が指すパスにソースがあればそれを使う (ローカル開発用)
#   2. 無ければ GitHub (wtnbgo/live2d) から取得してビルドする
set(LIVE2DLIB_FOLDER "$ENV{LIVE2DLIB_FOLDER}" CACHE PATH "Path to local live2d driver library source")
if(LIVE2DLIB_FOLDER AND EXISTS "${LIVE2DLIB_FOLDER}/CMakeLists.txt")
    message(STATUS "live2d driver: using local source at ${LIVE2DLIB_FOLDER}")
    add_subdirectory(${LIVE2DLIB_FOLDER} ${CMAKE_CURRENT_BINARY_DIR}/live2ddriver)
else()
    message(STATUS "live2d driver: fetching from https://github.com/wtnbgo/live2d.git")
    include(FetchContent)
    FetchContent_Declare(live2ddriver
        GIT_REPOSITORY https://github.com/wtnbgo/live2d.git
        GIT_TAG main
        GIT_SUBMODULES_RECURSE ON
    )
    FetchContent_MakeAvailable(live2ddriver)
endif()
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
