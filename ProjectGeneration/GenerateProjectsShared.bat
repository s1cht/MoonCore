@echo off
setlocal

REM Always work relative to this script's folder, regardless of where it's launched from
cd /D "%~dp0"

set "PREMAKE_VERSION=5.0.0"
set "PREMAKE_DIR=%USERPROFILE%"
set "PREMAKE_EXE=%PREMAKE_DIR%\premake5.exe"
set "PREMAKE_URL=https://github.com/premake/premake-core/releases/download/v%PREMAKE_VERSION%/premake-%PREMAKE_VERSION%-windows.zip"
set "PREMAKE_ZIP=%TEMP%\premake-%PREMAKE_VERSION%-windows.zip"

if not exist "%PREMAKE_EXE%" (
    echo premake5.exe not found, downloading v%PREMAKE_VERSION%...
    if not exist "%PREMAKE_DIR%" mkdir "%PREMAKE_DIR%"

    curl -L --fail -o "%PREMAKE_ZIP%" "%PREMAKE_URL%" || goto :download_failed
    tar -xf "%PREMAKE_ZIP%" -C "%PREMAKE_DIR%"        || goto :extract_failed
    del "%PREMAKE_ZIP%"

    if not exist "%PREMAKE_EXE%" goto :extract_failed
)

REM Prepend to PATH for this script only (setlocal makes it vanish afterwards)
set "PATH=%PREMAKE_DIR%;%PATH%"

pushd "%~dp0.."
premake5 --file="solution.lua" vs2026 --verbose
popd
pause
exit /b 0

:download_failed
echo [ERROR] Failed to download %PREMAKE_URL%
pause
exit /b 1

:extract_failed
echo [ERROR] Failed to extract premake to %PREMAKE_DIR%
pause
exit /b 1