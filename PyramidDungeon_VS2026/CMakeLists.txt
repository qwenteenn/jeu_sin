cmake_minimum_required(VERSION 3.20)

project(PyramidDungeon
    VERSION 1.0.0
    DESCRIPTION "3D solo dungeon crawler roguelite in a buried pyramid"
    LANGUAGES CXX
)

include(FetchContent)

# CMake 4.x peut devenir pénible avec de vieux CMakeLists de dépendances.
# Raylib 5.5 affiche parfois un warning de compatibilité, donc on donne une version minimale propre.
if (NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.10 CACHE STRING "Minimum policy version for dependencies" FORCE)
endif()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RSHAPES ON CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RTEXTURES ON CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RTEXT ON CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RMODELS ON CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RAUDIO ON CACHE BOOL "" FORCE)

option(PYRAMID_USE_SYSTEM_RAYLIB "Use an already installed raylib package instead of downloading it" OFF)

if (PYRAMID_USE_SYSTEM_RAYLIB)
    find_package(raylib CONFIG QUIET)
endif()

if (NOT TARGET raylib)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(PyramidDungeon ${PROJECT_SOURCES})

target_include_directories(PyramidDungeon PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(PyramidDungeon PRIVATE raylib)

# Important pour VS 2026 Insider : ne pas utiliser CXX_STANDARD 18.
# C++18 n'existe pas. Oui, magnifique civilisation technique.
# On force directement /std:c++17 avec MSVC pour éviter l'erreur CXX18.
if (MSVC)
    target_compile_options(PyramidDungeon PRIVATE /std:c++17 /utf-8 /W3 /permissive- /Zc:__cplusplus)
    target_link_options(PyramidDungeon PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
    target_compile_definitions(PyramidDungeon PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PyramidDungeon)
else()
    target_compile_features(PyramidDungeon PRIVATE cxx_std_17)
    target_compile_options(PyramidDungeon PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET PyramidDungeon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:PyramidDungeon>/assets"
)

install(TARGETS PyramidDungeon RUNTIME DESTINATION .)
install(DIRECTORY assets DESTINATION .)
