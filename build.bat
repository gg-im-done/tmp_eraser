@echo off
set SCRIPT_PATH=%~dp0
set OUT_DIR=%SCRIPT_PATH%\Build\
msbuild %SCRIPT_PATH%\tmp_eraser.sln -p:Configuration=Release;Platform=x64;OutDir=%OUT_DIR% -noLogo -verbosity:m
set BUILD_RESULT=%ERRORLEVEL%
if %BUILD_RESULT%==0 (
echo ---Start---
Start "" "%OUT_DIR%\tmp_eraser.exe"
) else(pause)