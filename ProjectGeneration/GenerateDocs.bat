@echo off
setlocal

REM Always work relative to this script's folder, regardless of where it's launched from
cd /D "%~dp0.."

where doxygen >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Doxygen was not found in PATH.
    echo Please install Doxygen from https://www.doxygen.nl/download.html and add it to your PATH.
    pause
    exit /b 1
)

echo Generating MoonCore Documentation...
doxygen Doxyfile
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Doxygen documentation generation failed.
    pause
    exit /b 1
)

echo.
echo Documentation successfully generated!
echo Open docs\html\index.html in your browser.
pause
exit /b 0
