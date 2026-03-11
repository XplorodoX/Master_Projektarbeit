cmake_minimum_required(VERSION 3.20)
project(LeconProjekt VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_HEADLESS_RUNNER "Build the fast headless simulator app" ON)
option(BUILD_RAYLIB_CLIENT "Build the raylib playable client" ON)
option(BUILD_SDL_CLIENT "Build the SDL2 playable client (legacy fallback)" OFF)
option(BUILD_PYTHON_BINDINGS "Build Python bindings for Gymnasium" ON)
option(LECON_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)

add_library(lecon_core
    src/core/world.cpp
    src/core/simulation.cpp
)

target_include_directories(lecon_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(lecon_core PUBLIC cxx_std_20)
lecon_set_warnings(lecon_core ${LECON_WARNINGS_AS_ERRORS})

if(BUILD_HEADLESS_RUNNER)
    add_executable(lecon_headless src/apps/headless_main.cpp)
    target_link_libraries(lecon_headless PRIVATE lecon_core)
    lecon_set_warnings(lecon_headless ${LECON_WARNINGS_AS_ERRORS})
endif()

if(BUILD_RAYLIB_CLIENT)
    find_package(raylib QUIET)

    set(LECON_RAYLIB_TARGET "")
    if(TARGET raylib)
        set(LECON_RAYLIB_TARGET raylib)
    elseif(TARGET raylib::raylib)
        set(LECON_RAYLIB_TARGET raylib::raylib)
    else()
        find_library(RAYLIB_LIBRARY NAMES raylib)
        find_path(RAYLIB_INCLUDE_DIR raylib.h)

        if(RAYLIB_LIBRARY AND RAYLIB_INCLUDE_DIR)
            add_library(raylib_manual UNKNOWN IMPORTED)
            set_target_properties(raylib_manual PROPERTIES
                IMPORTED_LOCATION "${RAYLIB_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
            )
            set(LECON_RAYLIB_TARGET raylib_manual)
        endif()
    endif()

    if(LECON_RAYLIB_TARGET)
        add_executable(lecon_client src/client/raylib_main.cpp)
        target_link_libraries(lecon_client PRIVATE lecon_core ${LECON_RAYLIB_TARGET})
        lecon_set_warnings(lecon_client ${LECON_WARNINGS_AS_ERRORS})
    else()
        message(WARNING "raylib not found. Skipping lecon_client target.")
    endif()
elseif(BUILD_SDL_CLIENT)
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        add_executable(lecon_client src/client/sdl_main.cpp)
        target_link_libraries(lecon_client PRIVATE lecon_core SDL2::SDL2)
        lecon_set_warnings(lecon_client ${LECON_WARNINGS_AS_ERRORS})
    else()
        message(WARNING "SDL2 not found. Skipping lecon_client target.")
    endif()
endif()

if(BUILD_PYTHON_BINDINGS)
    set(LECON_VENV_PYTHON "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python")
    if(EXISTS "${LECON_VENV_PYTHON}")
        set(Python3_EXECUTABLE "${LECON_VENV_PYTHON}" CACHE FILEPATH "Preferred Python interpreter for bindings" FORCE)
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

        pybind11_add_module(lecon_sim src/python/py_module.cpp)
        target_link_libraries(lecon_sim PRIVATE lecon_core)
        target_include_directories(lecon_sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        set_target_properties(lecon_sim PROPERTIES PREFIX "" SUFFIX ".so")
        lecon_set_warnings(lecon_sim ${LECON_WARNINGS_AS_ERRORS})
    else()
        message(WARNING "Python3 dev headers not found. Skipping lecon_sim module.")
    endif()
endif()
