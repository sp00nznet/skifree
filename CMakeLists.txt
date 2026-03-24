cmake_minimum_required(VERSION 3.16)
project(skifree C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_C_STANDARD 99)

# --- Find SDL2 and friends ---
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

# --- Embed resources at build time ---
file(GLOB resource_files "resources/*.bmp" "resources/*.fon" "resources/*.ttf" "resources/*.ICO")

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
if(APPLE)
    # Generate icns from ICO if imagemagick is available
    find_program(CONVERT_CMD convert)
    if(CONVERT_CMD)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ICONSKI.png
            COMMAND ${CONVERT_CMD}
            ARGS -flatten ${CMAKE_SOURCE_DIR}/resources/ICONSKI.ICO ${CMAKE_CURRENT_BINARY_DIR}/ICONSKI.PNG
            DEPENDS ${CMAKE_SOURCE_DIR}/resources/ICONSKI.ICO
        )
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ski32.icns
            COMMAND ${CMAKE_SOURCE_DIR}/tools/make-osx-icns.sh
            DEPENDS ICONSKI.png
        )
        set(SKIFREE_ICON ${CMAKE_CURRENT_BINARY_DIR}/ski32.icns)
        set_source_files_properties(${SKIFREE_ICON} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
    endif()

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SKIFREE_ICON})
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER com.skifree.recomp
        MACOSX_BUNDLE_BUNDLE_NAME "SkiFree"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
elseif(WIN32)
    add_executable(${PROJECT_NAME} WIN32)
else()
    add_executable(${PROJECT_NAME})
endif()

target_sources(${PROJECT_NAME} PRIVATE
    src/skifree.c
    src/resources.c
    src/config.c
    src/sound.c
    src/ai.c
    src/physics.c
    src/replay.c
    src/net.c
    "${CMAKE_CURRENT_BINARY_DIR}/embedded_resources.h"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${CMAKE_CURRENT_BINARY_DIR}
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
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

# --- Link libraries ---
# Handle different SDL2 CMake configurations
if(TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
elseif(SDL2_LIBRARIES)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()

if(TARGET SDL2_image::SDL2_image)
    target_link_libraries(${PROJECT_NAME} SDL2_image::SDL2_image)
elseif(SDL2IMAGE_LIBRARY)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2IMAGE_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2IMAGE_LIBRARY})
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
