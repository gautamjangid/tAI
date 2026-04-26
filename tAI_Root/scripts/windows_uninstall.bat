@echo off
REM tAI Windows Uninstaller
REM Removes tAI binary and configuration files from the system

setlocal enabledelayedexpansion

REM Define colors and output styles
cls
echo.
echo ====================================
echo     tAI Windows Uninstaller
echo ====================================
echo.

REM Check for admin privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo WARNING: This script should be run with Administrator privileges.
    echo Please run Command Prompt as Administrator and try again.
    echo.
    pause
    exit /b 1
)

REM Define installation paths
set "INSTALL_DIR=%ProgramFiles%\tAI"
set "CONFIG_PATH=%USERPROFILE%\.tAI"
set "PATH_ENTRY=%ProgramFiles%\tAI"

REM Track what was removed
set "REMOVED_COUNT=0"

REM Remove executable if it exists
if exist "%INSTALL_DIR%\tAI.exe" (
    echo Removing tAI.exe from %INSTALL_DIR%...
    del /q "%INSTALL_DIR%\tAI.exe" >nul 2>&1
    if !errorLevel! equ 0 (
        echo [OK] tAI.exe removed
        set /a "REMOVED_COUNT+=1"
    ) else (
        echo [ERROR] Failed to remove tAI.exe. Ensure you have administrator privileges.
    )
)

REM Remove directory if it exists and is empty
if exist "%INSTALL_DIR%" (
    rmdir "%INSTALL_DIR%" >nul 2>&1
    if !errorLevel! equ 0 (
        echo [OK] Installation directory removed
    )
)

REM Ask about removing configuration
if exist "%CONFIG_PATH%" (
    echo.
    echo Configuration directory found at: %CONFIG_PATH%
    set /p "REMOVE_CONFIG=Do you want to remove your configuration? (y/n): "

    if /i "!REMOVE_CONFIG!"=="y" (
        echo Removing configuration directory...
        rmdir /s /q "%CONFIG_PATH%" >nul 2>&1
        if !errorLevel! equ 0 (
            echo [OK] Configuration directory removed
            set /a "REMOVED_COUNT+=1"
        ) else (
            echo [ERROR] Failed to remove configuration directory.
        )
    ) else (
        echo Configuration directory preserved at: %CONFIG_PATH%
    )
) else (
    echo [INFO] No configuration directory found at %CONFIG_PATH%
)

REM Summary
echo.
echo ====================================
echo     Uninstallation Complete
echo ====================================
echo.

if !REMOVED_COUNT! gtr 0 (
    echo tAI has been successfully uninstalled.
    echo Thank you for using tAI!
) else (
    echo No tAI files were found to remove.
    echo You may have already uninstalled tAI.
)

echo.
echo Press any key to exit...
pause >nul
exit /b 0
