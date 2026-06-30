@echo off
REM --- Lyudishki Windows Build + Installer ---
REM
REM Prerequisites:
REM   - Qt 6 installed (e.g. C:\Qt\6.9.2\msvc2022_64)
REM   - CMake in PATH
REM   - Visual Studio Build Tools (cl.exe)
REM   - Inno Setup 6 (iscc.exe in PATH or in standard location)
REM
REM Usage: scripts\build_windows.bat [QT_DIR]
REM   Example: scripts\build_windows.bat C:\Qt\6.9.2\msvc2022_64

setlocal enabledelayedexpansion

set QT_DIR=%1
if "%QT_DIR%"=="" (
    echo Usage: build_windows.bat ^<QT_DIR^>
    echo   Example: build_windows.bat C:\Qt\6.9.2\msvc2022_64
    exit /b 1
)

set PROJECT_DIR=%~dp0..
set BUILD_DIR=%PROJECT_DIR%\build

echo === Lyudishki Windows Build ===
echo Qt: %QT_DIR%
echo.

REM Step 1: CMake configure
echo [1/4] Configuring...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"
cmake "%PROJECT_DIR%" -DCMAKE_PREFIX_PATH="%QT_DIR%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (echo CMake configure failed & exit /b 1)

REM Step 2: Build
echo [2/4] Building...
cmake --build . --config Release
if errorlevel 1 (echo Build failed & exit /b 1)

REM Step 3: Deploy Qt
echo [3/4] Running windeployqt...
"%QT_DIR%\bin\windeployqt.exe" "%BUILD_DIR%\Release\Lyudishki.exe"
if errorlevel 1 (echo windeployqt failed & exit /b 1)

REM Step 4: Create installer
echo [4/4] Building installer...
set ISCC=iscc.exe
where iscc >nul 2>&1
if errorlevel 1 (
    if exist "C:\Program Files (x86)\Inno Setup 6\iscc.exe" (
        set ISCC="C:\Program Files (x86)\Inno Setup 6\iscc.exe"
    ) else (
        echo WARNING: Inno Setup not found. Skipping installer.
        echo   Install from: https://jrsoftware.org/isdl.php
        goto done
    )
)

%ISCC% "%PROJECT_DIR%\scripts\lyudishki_installer.iss"
if errorlevel 1 (echo Installer build failed & exit /b 1)

:done
echo.
echo === Done ===
echo Executable: %BUILD_DIR%\Release\Lyudishki.exe
if exist "%BUILD_DIR%\Lyudishki-Setup-*.exe" (
    echo Installer: %BUILD_DIR%\Lyudishki-Setup-1.0.0.exe
)
