cmake_minimum_required(VERSION 3.6)
project(SKIFREE C)

# SDL2's source is staged into jni/SDL by scripts/build_android.sh (gitignored).
add_subdirectory(SDL)

# The lite game (SKIFREE_LITE: SDL2 alone). The source list is LITE_SRC from
# scripts/env.sh, which build_android.sh exports as SKIFREE_LITE_SRC, so it
# stays in one place. SDL_main.h renames main() to SDL_main on Android, which
# is what SDLActivity loads from libmain.so.
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
if(NOT DEFINED ENV{SKIFREE_LITE_SRC})
    message(FATAL_ERROR "SKIFREE_LITE_SRC is unset: build with scripts/build_android.sh")
endif()
string(REGEX REPLACE "[ \t\r\n]+" ";" SRCS "$ENV{SKIFREE_LITE_SRC}")
list(FILTER SRCS EXCLUDE REGEX "^$")
list(TRANSFORM SRCS PREPEND "${ROOT}/")

add_library(main SHARED ${SRCS})
target_include_directories(main BEFORE PRIVATE
    ${ROOT}/src/lite ${ROOT}/src ${ROOT}/third_party/cjson ${ROOT}/build-gen)
target_compile_definitions(main PRIVATE SKIFREE_LITE)
target_compile_options(main PRIVATE -w)
target_link_libraries(main SDL2 m)
