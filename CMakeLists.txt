cmake_minimum_required(VERSION 3.21)
project(skifree C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

# --- Find SDL2 and friends ---
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

# --- Embed resources at build time ---
file(GLOB resource_files "resources/*")

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/embedded_resources.h"
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
        "-DRESOURCES_GLOB=resources/*"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedResources.cmake"
    DEPENDS ${resource_files}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Embedding game resources..."
)

# --- Main executable ---
add_executable(${PROJECT_NAME} WIN32)

target_sources(${PROJECT_NAME} PRIVATE
    src/skifree.c
    src/resources.c
    src/config.c
    src/sound.c
    src/ai.c
    src/physics.c
    src/replay.c
    src/net.c
    src/win32_menu.c
    src/win32_dialogs.c
    src/saveload.c
    src/input_bind.c
    src/menu_gui.cpp
    third_party/cjson/cJSON.c
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_sdl2.cpp
    third_party/imgui/backends/imgui_impl_sdlrenderer2.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/embedded_resources.h"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/cjson
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/third_party/imgui/backends
)

# --- Compiler settings ---
if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE /subsystem:windows /ENTRY:mainCRTStartup)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        -D_CRT_SECURE_NO_WARNINGS
        -DSDL_MAIN_HANDLED
        -DWIN32_LEAN_AND_MEAN
    )
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /bigobj)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wno-unused-variable)
endif()

# --- Platform-specific ---
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32 comdlg32)
endif()

# --- Link libraries ---
# Handle different SDL2 CMake configurations
if(TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
elseif(SDL2_LIBRARIES)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()

if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(${PROJECT_NAME} SDL2_ttf::SDL2_ttf)
elseif(SDL2TTF_LIBRARY)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2TTF_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2TTF_LIBRARY})
endif()

if(TARGET SDL2_mixer::SDL2_mixer)
    target_link_libraries(${PROJECT_NAME} SDL2_mixer::SDL2_mixer)
elseif(SDL2MIXER_LIBRARY)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2MIXER_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2MIXER_LIBRARY})
endif()

# Windows: the SDL DLLs beside skifree.exe, so it runs from the build folder
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS)
endif()
