@echo off
REM Quick helper to build bindings, train DQN and/or play back a model with visualization.
REM Usage:
REM   scripts\start_dqn_play.bat [train|play|both] [timesteps] [model_path] [seed]
REM Examples:
REM   scripts\start_dqn_play.bat play                # just start playback (uses default model)
REM   scripts\start_dqn_play.bat train 200000        # train DQN for 200k steps
REM   scripts\start_dqn_play.bat both 100000         # train then play

setlocal enabledelayedexpansion

set "ROOT=%~dp0.."
set "VENV_ACT=%ROOT%\.venv\Scripts\activate.bat"
set "PYTHON=%ROOT%\.venv\Scripts\python.exe"
set "BUILD_DIR=%ROOT%\build"
set "REQ_FILE=%ROOT%\python\requirements.txt"
set "REQ_MARKER=%ROOT%\.venv\.requirements_installed"

if not exist "%VENV_ACT%" (
    echo.
    echo ERROR: Virtual environment not found at %VENV_ACT%
    echo Please create the virtual environment and install requirements (see README).
    echo.
    exit /b 1
)

REM Activate venv
call "%VENV_ACT%"

REM Parse arguments with defaults
set "action=%1"
if "!action!"=="" set "action=both"

set "timesteps=%2"
if "!timesteps!"=="" set "timesteps=100000"

set "model_path=%3"
if "!model_path!"=="" set "model_path=best_models_dqn/best_model.zip"

set "seed=%4"
if "!seed!"=="" set "seed=42"

REM ---------------------------------------------------------------------------
REM Build Python bindings
REM ---------------------------------------------------------------------------

:build_bindings
if exist "%ROOT%\python\stoneforge_sim.pyd" (
    echo Python bindings already present: python\stoneforge_sim.pyd
    goto :build_complete
)

REM Install requirements once if present
if exist "%REQ_FILE%" if exist "%VENV_ACT%" if not exist "%REQ_MARKER%" (
    echo Installing Python requirements into venv...
    "%PYTHON%" -m pip install -r "%REQ_FILE%"
    if errorlevel 1 (
        echo ERROR: Failed to install requirements
        exit /b 1
    )
    type nul > "%REQ_MARKER%"
)

echo Building Python bindings ^(stoneforge_sim^)...
cmake -S "%ROOT%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
if errorlevel 1 (
    echo ERROR: CMake configure failed
    exit /b 1
)

cmake --build "%BUILD_DIR%" --target stoneforge_sim -j 4
if errorlevel 1 (
    echo ERROR: CMake build failed
    exit /b 1
)

if exist "%BUILD_DIR%\Release\stoneforge_sim.pyd" (
    copy /Y "%BUILD_DIR%\Release\stoneforge_sim.pyd" "%ROOT%\python\stoneforge_sim.pyd"
    echo Copied stoneforge_sim.pyd to python\
) else if exist "%BUILD_DIR%\stoneforge_sim.pyd" (
    copy /Y "%BUILD_DIR%\stoneforge_sim.pyd" "%ROOT%\python\stoneforge_sim.pyd"
    echo Copied stoneforge_sim.pyd to python\
) else (
    echo ERROR: stoneforge_sim.pyd build failed or not found
    exit /b 1
)

:build_complete

REM ---------------------------------------------------------------------------
REM Execute requested action
REM ---------------------------------------------------------------------------

if /i "!action!"=="train" (
    echo Training DQN for !timesteps! timesteps...
    "%PYTHON%" python\train.py --algo dqn --timesteps !timesteps!
    exit /b !errorlevel!
)

if /i "!action!"=="play" (
    echo Playing model: !model_path!
    "%PYTHON%" python\ai_play.py --model "!model_path!" --seed !seed! --speed 1.0
    exit /b !errorlevel!
)

if /i "!action!"=="both" (
    echo Training DQN for !timesteps! timesteps...
    "%PYTHON%" python\train.py --algo dqn --timesteps !timesteps!
    if errorlevel 1 (
        echo ERROR: Training failed
        exit /b 1
    )
    
    echo Playing trained model...
    "%PYTHON%" python\ai_play.py --model "!model_path!" --seed !seed! --speed 1.0
    exit /b !errorlevel!
)

echo.
echo ERROR: Unknown action "!action!" (use: train, play, or both)
echo.
exit /b 1
