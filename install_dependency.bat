@echo off

REM Install a specified dependency locally for find_package.

REM Check if base path argument is provided
if "%~1"=="" (
    echo Usage: install_dependency.bat [BASE_PATH]
    exit /b 1
)

REM Set directories
set SOURCE_DIR=%~dp0/lib/%~1
set BUILD_DIR=%~dp0/lib/build/%~1
set INSTALL_DIR=%~dp0/lib/install/%~1

echo Source dir: %SOURCE_DIR%
echo Build dir: %BUILD_DIR%
echo Install dir: %INSTALL_DIR%

REM Configure
REM TODO: move arguments elsewhere
cmake -S "%SOURCE_DIR%" -B "%BUILD_DIR%" -DGLSLANG_ENABLE_INSTALL=ON -DENABLE_OPT=OFF -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%"

REM Build
cmake --build "%BUILD_DIR%" --config Debug

REM Install
cmake --install "%BUILD_DIR%" --config Debug

echo Install complete for %SOURCE_DIR% at "%INSTALL_DIR%"
