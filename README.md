# Aka Engine

Small 2D game engine, mainly built for personal educational purpose. Its main target is pixel art game, and its pipeline is built around [Ogmo editor](https://ogmo-editor-3.github.io/) for level creation and [aseprite](https://www.aseprite.org/) for sprite creation (even though photoshop can do the work).

Aka stand for red in japanese (赤) and there is no particular reason for this name, we just need one !

## Build
-   Run --recursive with git clone or git submodule init / update to get all dependencies.
-   This project was build using Visual Studio 2019 under Windows 10. Others versions have not been tested.

## RoadMap
-   More robust physic engine or use third party like [Box2D](https://box2d.org/)
-   Add DirectX support
-   Add multiple shapes for colliders
-   Use an atlas for font rendering
-   Use stb_true_type for less big dependencies (freetype)
-   Parse aseprite files directly
-   Project to CMake to support build for Linux
-   Add support for joystick
-   Compatibility with linux
-   Add sound support (OpenAL / Juce)
-   Let's work on a game !
