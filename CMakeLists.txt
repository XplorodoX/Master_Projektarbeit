cmake_minimum_required(VERSION 3.20)
project(StoneforgeFrontier VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_HEADLESS_RUNNER "Build the fast headless simulator app" ON)
option(BUILD_RAYLIB_CLIENT "Build the raylib playable client" ON)
option(BUILD_SDL_CLIENT "Build the SDL2 playable client (legacy fallback)" OFF)
option(BUILD_PYTHON_BINDINGS "Build Python bindings for Gymnasium" ON)
option(STONEFORGE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)

add_library(stoneforge_core
    src/core/world.cpp
    src/core/simulation.cpp
)

target_include_directories(stoneforge_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(stoneforge_core PUBLIC cxx_std_20)
stoneforge_set_warnings(stoneforge_core ${STONEFORGE_WARNINGS_AS_ERRORS})

if(BUILD_HEADLESS_RUNNER)
    add_executable(stoneforge_headless src/apps/headless_main.cpp)
    target_link_libraries(stoneforge_headless PRIVATE stoneforge_core)
    stoneforge_set_warnings(stoneforge_headless ${STONEFORGE_WARNINGS_AS_ERRORS})
endif()

if(BUILD_RAYLIB_CLIENT)
    find_package(raylib QUIET)

    set(STONEFORGE_RAYLIB_TARGET "")
    if(TARGET raylib)
        set(STONEFORGE_RAYLIB_TARGET raylib)
    elseif(TARGET raylib::raylib)
        set(STONEFORGE_RAYLIB_TARGET raylib::raylib)
    else()
        find_library(RAYLIB_LIBRARY NAMES raylib)
        find_path(RAYLIB_INCLUDE_DIR raylib.h)

        if(RAYLIB_LIBRARY AND RAYLIB_INCLUDE_DIR)
            add_library(raylib_manual UNKNOWN IMPORTED)
            set_target_properties(raylib_manual PROPERTIES
                IMPORTED_LOCATION "${RAYLIB_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
            )
            set(STONEFORGE_RAYLIB_TARGET raylib_manual)
        endif()
    endif()

    if(NOT STONEFORGE_RAYLIB_TARGET)
        include(FetchContent)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
        set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
        set(SUPPORT_MODULE_RAUDIO OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_MOD OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            raylib
            URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(raylib)

        if(TARGET raylib)
            set(STONEFORGE_RAYLIB_TARGET raylib)
        elseif(TARGET raylib::raylib)
            set(STONEFORGE_RAYLIB_TARGET raylib::raylib)
        endif()

        if(TARGET raylib)
            if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
                # Suppress known GCC false positives from bundled third-party decoder sources.
                target_compile_options(raylib PRIVATE -Wno-stringop-overflow -Wno-array-bounds)
            endif()
        endif()
    endif()

    if(STONEFORGE_RAYLIB_TARGET)
        add_executable(stoneforge_client
            src/client/raylib_main.cpp
            src/client/render_engine.cpp
            src/client/render_fx.cpp
            src/client/render_ui.cpp
        )
        target_link_libraries(stoneforge_client PRIVATE stoneforge_core ${STONEFORGE_RAYLIB_TARGET})
        stoneforge_set_warnings(stoneforge_client ${STONEFORGE_WARNINGS_AS_ERRORS})
    else()
        message(WARNING "raylib not found. Skipping stoneforge_client target.")
    endif()
elseif(BUILD_SDL_CLIENT)
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        add_executable(stoneforge_client src/client/sdl_main.cpp)
        target_link_libraries(stoneforge_client PRIVATE stoneforge_core SDL2::SDL2)
        stoneforge_set_warnings(stoneforge_client ${STONEFORGE_WARNINGS_AS_ERRORS})
    else()
        message(WARNING "SDL2 not found. Skipping stoneforge_client target.")
    endif()
endif()

if(BUILD_PYTHON_BINDINGS)
    set(STONEFORGE_VENV_PYTHON "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python")
    if(EXISTS "${STONEFORGE_VENV_PYTHON}")
        set(Python3_EXECUTABLE "${STONEFORGE_VENV_PYTHON}" CACHE FILEPATH "Preferred Python interpreter for bindings" FORCE)
    endif()

    find_package(Python3 COMPONENTS Interpreter Development.Module QUIET)
    if(Python3_FOUND)
        include(FetchContent)
        set(PYBIND11_FINDPYTHON ON)
        FetchContent_Declare(
            pybind11
            URL https://github.com/pybind/pybind11/archive/refs/tags/v2.13.6.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(pybind11)

        pybind11_add_module(stoneforge_sim src/python/py_module.cpp)
        target_link_libraries(stoneforge_sim PRIVATE stoneforge_core)
        target_include_directories(stoneforge_sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        set_target_properties(stoneforge_sim PROPERTIES PREFIX "" SUFFIX ".so")
        stoneforge_set_warnings(stoneforge_sim ${STONEFORGE_WARNINGS_AS_ERRORS})
    else()
        message(WARNING "Python3 dev headers not found. Skipping stoneforge_sim module.")
    endif()
endif()
