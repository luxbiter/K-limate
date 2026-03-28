@echo off
setlocal EnableDelayedExpansion

echo.
echo  ================================================
echo   Klimate - Nintendo Switch Weather Overlay
echo   Build Script for Windows
echo  ================================================
echo.

REM -- Check devkitPro ----------------------------------------------------------
if "%DEVKITPRO%"=="" set "DEVKITPRO=C:\devkitPro"
set "BASH=%DEVKITPRO%\msys2\usr\bin\bash.exe"

if not exist "%BASH%" (
    echo  [X] devkitPro not found at: %DEVKITPRO%
    echo.
    echo  How to install devkitPro:
    echo  1. Download devkitProUpdater from:
    echo     https://github.com/devkitPro/installer/releases
    echo.
    echo  2. Run the installer and select:
    echo     - switch-dev
    echo     - switch-libnx
    echo     - switch-curl
    echo     - switch-mbedtls
    echo     - switch-zlib
    echo     - switch-libtesla
    echo.
    echo  3. Re-run this script after installation.
    echo.
    pause
    exit /b 1
)
echo  [OK] devkitPro found: %DEVKITPRO%

REM -- Download json.hpp if missing ---------------------------------------------
if not exist "include\json.hpp" (
    echo  [..] Downloading nlohmann/json.hpp...
    curl -fsSL -o "include\json.hpp" "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
    if errorlevel 1 (
        echo  [X] Download failed. Check your internet connection.
        pause
        exit /b 1
    )
    echo  [OK] json.hpp downloaded
) else (
    echo  [OK] json.hpp already exists
)

REM -- Install required packages ------------------------------------------------
echo  [..] Installing required Switch packages...
echo       (This may take a while on first run)
"%BASH%" -l -c "dkp-pacman -S --needed --noconfirm switch-dev switch-libnx switch-curl switch-mbedtls switch-zlib switch-libtesla"
if errorlevel 1 (
    echo  [X] Package installation failed.
    pause
    exit /b 1
)
echo  [OK] Packages ready

REM -- Build --------------------------------------------------------------------
echo  [..] Building...

set "WIN_DIR=%~dp0"
if "%WIN_DIR:~-1%"=="\" set "WIN_DIR=%WIN_DIR:~0,-1%"

"%BASH%" -l -c "cd \"$(cygpath '%WIN_DIR%')\" && make 2>&1"

if errorlevel 1 (
    echo.
    echo  [X] Build failed! See error messages above.
    echo.
    pause
    exit /b 1
)

echo.
echo  ================================================
echo   [OK] Build successful! weather_overlay.ovl
echo  ================================================
echo.
echo  Copy to SD card:
echo    weather_overlay.ovl  -->  SD:/switch/.overlays/
echo.
echo  Config file required:
echo    SD:/config/weather.json
echo    (See weather.json.example for reference)
echo.
pause
