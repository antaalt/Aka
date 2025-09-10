@echo off

REM Install all required dependencies.

REM All entries to build and install
set ENTRIES=glfw freetype glslang SPIRV-Cross stb utf8 rtaudio renderdoc miniz minimp3 json imgui geometry

REM Loop over each entry
for %%F in (%ENTRIES%) do (
    echo -----------------------------
    echo Building %%F
    if exist "%~dp0/install_dependency.bat" (
        call "%~dp0/install_dependency.bat" %* "%%F"
    ) else (
        echo install_dependency.bat not found for %%F
    )
)
pause