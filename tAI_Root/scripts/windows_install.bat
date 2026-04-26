@echo off
REM tAI Installation Script for Windows
REM Requires: Administrator privileges, built binary in build\ directory
REM Usage: Right-click and "Run as administrator"

setlocal enabledelayedexpansion

REM Color codes (using findstr for colored output)
for /F %%A in ('copy /Z "%~f0" nul') do set "ESC=%%A"

echo.
echo %ESC%[92m=== tAI Installation for Windows ===%ESC%[0m
echo.

REM Check for administrator privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo %ESC%[91mError: This script must be run as Administrator%ESC%[0m
    echo.
    echo Please:
    echo 1. Right-click on this file
    echo 2. Select "Run as administrator"
    echo.
    pause
    exit /b 1
)

REM Get script directory
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%..\\"
set "BUILD_DIR=%PROJECT_ROOT%build"
set "BINARY_NAME=tAI.exe"
set "BINARY_PATH=%BUILD_DIR%\Release\%BINARY_NAME%"

REM Alternative build path (in case Release is at build level)
if not exist "!BINARY_PATH!" (
    set "BINARY_PATH=%BUILD_DIR%\%BINARY_NAME%"
)

echo Checking for binary at: !BINARY_PATH!
echo.

REM Check if binary exists
if not exist "!BINARY_PATH!" (
    echo %ESC%[91mError: Binary not found at !BINARY_PATH!%ESC%[0m
    echo.
    echo Please build tAI first using:
    echo   1. Open Visual Studio Developer Command Prompt
    echo   2. Navigate to the project root
    echo   3. Run these commands:
    echo      mkdir build
    echo      cd build
    echo      cmake .. -DCMAKE_BUILD_TYPE=Release
    echo      cmake --build . --config Release
    echo.
    pause
    exit /b 1
)

echo %ESC%[92m✓ Binary found!%ESC%[0m
echo.

REM Choose installation directory
set "INSTALL_DIR=%ProgramFiles%\tAI"
echo Installation directory: %INSTALL_DIR%
echo.

REM Create installation directory if it doesn't exist
if not exist "!INSTALL_DIR!" (
    echo Creating installation directory...
    mkdir "!INSTALL_DIR!"
    if !errorlevel! neq 0 (
        echo %ESC%[91mError: Failed to create directory !INSTALL_DIR!%ESC%[0m
        echo Check that you have administrator privileges.
        pause
        exit /b 1
    )
)

REM Copy binary
echo Copying binary...
copy "!BINARY_PATH!" "!INSTALL_DIR!\%BINARY_NAME%" >nul
if !errorlevel! neq 0 (
    echo %ESC%[91mError: Failed to copy binary%ESC%[0m
    pause
    exit /b 1
)
echo %ESC%[92m✓ Binary copied%ESC%[0m

REM Add to PATH if not already there
echo.
echo Checking if !INSTALL_DIR! is in PATH...

for /f "tokens=2*" %%A in (
    'reg query HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment /v PATH'
) do (
    set "CURRENT_PATH=%%B"
)

echo Current PATH: !CURRENT_PATH!
echo.

setlocal enabledelayedexpansion
echo !CURRENT_PATH! | findstr /I "!INSTALL_DIR!" >nul
if !errorlevel! equ 0 (
    echo %ESC%[92m✓ Installation directory already in PATH%ESC%[0m
) else (
    echo Adding installation directory to PATH...

    REM Backup current PATH
    set "NEW_PATH=!CURRENT_PATH!;!INSTALL_DIR!"

    REM Update system PATH
    reg add HKLM\SYSTEM\CurrentControlSet\Control\Session^Manager\Environment /v PATH /d "!NEW_PATH!" /f >nul

    if !errorlevel! equ 0 (
        echo %ESC%[92m✓ PATH updated successfully%ESC%[0m
    ) else (
        echo %ESC%[93m⚠ Warning: Could not update PATH automatically%ESC%[0m
        echo You may need to add !INSTALL_DIR! to your PATH manually.
    )
)

REM Create config directory
set "CONFIG_DIR=%USERPROFILE%\.tAI"
if not exist "!CONFIG_DIR!" (
    echo.
    echo Creating config directory at !CONFIG_DIR!...
    mkdir "!CONFIG_DIR!"
)

echo.
echo %ESC%[92m=== Installation Complete ===%ESC%[0m
echo.
echo tAI is now installed at: %INSTALL_DIR%
echo.
echo Next steps:
echo 1. Close and reopen Command Prompt to refresh PATH
echo 2. Test the installation by running: tAI --help
echo.
echo Quick start:
echo   tAI "What is artificial intelligence?"
echo   tAI -f "latest AI news"
echo   tAI -c "Write a Python function to reverse a string"
echo.
echo Configuration file location: %USERPROFILE%\.tAI\config.json
echo.
echo For more information, run: tAI --help
echo.

pause
exit /b 0
