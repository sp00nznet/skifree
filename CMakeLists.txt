cmake_minimum_required(VERSION 3.21)
project(skifree C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

# Lite: the build for consoles, Android and the browser. No ImGui menus, no
# network game and no SDL_ttf (src/lite/), so it needs SDL2 alone.
option(SKIFREE_LITE "Build without the menus, the network game and SDL_ttf" OFF)

find_package(SDL2 REQUIRED)
if(NOT SKIFREE_LITE)
    find_package(SDL2_ttf REQUIRED)
endif()

# --- The sprites and sounds from ski32.exe, embedded at build time ---
file(GLOB resource_files "resources/*" "sounds/*.wav")
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/embedded_resources.h"
    COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedResources.cmake"
    DEPENDS ${resource_files}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Embedding game resources..."
)

add_executable(${PROJECT_NAME} WIN32
    src/skifree.c
    src/resources.c
    src/config.c
    src/sound.c
    src/ai.c
    src/physics.c
    src/replay.c
    src/win32_menu.c
    src/win32_dialogs.c
    src/saveload.c
    src/input_bind.c
    third_party/cjson/cJSON.c
    "${CMAKE_CURRENT_BINARY_DIR}/embedded_resources.h"
)
target_include_directories(${PROJECT_NAME} PRIVATE src ${CMAKE_CURRENT_BINARY_DIR} third_party/cjson)

if(SKIFREE_LITE)
    target_sources(${PROJECT_NAME} PRIVATE src/lite/lite_stubs.c)
    target_include_directories(${PROJECT_NAME} BEFORE PRIVATE src/lite)
else()
    target_sources(${PROJECT_NAME} PRIVATE
        src/net.c
        src/menu_gui.cpp
        third_party/imgui/imgui.cpp
        third_party/imgui/imgui_draw.cpp
        third_party/imgui/imgui_tables.cpp
        third_party/imgui/imgui_widgets.cpp
        third_party/imgui/backends/imgui_impl_sdl2.cpp
        third_party/imgui/backends/imgui_impl_sdlrenderer2.cpp
    )
    target_include_directories(${PROJECT_NAME} PRIVATE third_party/imgui third_party/imgui/backends)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE /subsystem:windows /ENTRY:mainCRTStartup)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS SDL_MAIN_HANDLED WIN32_LEAN_AND_MEAN)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /bigobj)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wno-unused-variable)
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 comdlg32)
    # the SDL DLLs beside skifree.exe, so it runs from the build folder
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS)
endif()
