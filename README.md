## Overview
Alias Isolation is a mod for Alien: Isolation. It adds temporal anti-aliasing into the shipped game, and fixes a few small issues with the rendering.

The mod works by injecting itself into the executable, and hijacking D3D11 calls. It replaces a few shaders, and injects some of its own rendering.

This fork aims to maintain the mod, resolve existing issues, add new functionality and, eventually, port the mod to new platforms.

Suggestions for new features or pull requests for this mod are welcome!

## Installation instructions
Download the latest d3d11 x86 release of Ultimate ASI Loader from here:
https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/Win32-latest/d3d11-Win32.zip

Extract the following files from the `AliasIsolation.7z` archive, obtainable via the releases page, to the folder where Alien: Isolation is installed (you should be in a folder that has a file called `AI.exe`):
1. Extract the `mods` folder to your Alien: Isolation folder.
2. Extract `aliasIsolation.asi` and copy `d3d11.dll` to your Alien: Isolation folder.

> [!NOTE]
> If you're using Linux and want to play the game via an older version of Proton (version < Proton 8), you _might_ need to perform an extra step to let Proton know that you want it to use the ASI loader:
> * Add this launch option to the game, in Steam: `WINEDLLOVERRIDES="d3d11=n,b"`

## Video settings
These options must be set to the correct values for the mod to work properly.

| Option | Value |
| --- | --- |
| Anti-aliasing | SMAA T1x |
| Chromatic Aberration | Disabled |
| Motion Blur | Enabled |

## Runtime settings
To disable the mod at runtime, hit `Ctrl+Delete`. To re-enable it, hit `Ctrl+Insert`.

## Known issues
* No support for the unofficial Rift mode (could change in the future).
* RenderDoc, Epic Games Overlay, Windows Auto HDR, or any other software which also uses API hooking may break the rendering.
* SLI/CrossFire performance will probably be badly impacted, and depending on what the driver does, might also suffer from glitchy rendering.
* Some special effects can appear thinner or exhibit ghosting. Sparks are known to be eroded.
* While the motion tracker is in use, the Depth of Field shader seems to cause the tracker's LED highlights to appear, along the top of the screen.
* (Under investigation) The Epic Games Store release of the game crashes with a fatal error at "aliasIsolation_hookableOverlayRender".
* You are unable to move the mouse in the Alias Isolation UI while the game is unpaused. You can use the arrow keys to change settings in the menu, or pause the game which will unfreeze the mouse and allow you to adjust the settings.
* (Planned) No support for the integrated Cinematic Tools.
* Some UI backgrounds may become red in colour, which could be an issue with the original shaders.

## Building from source
You need to have Visual Studio 2022. Community Edition works fine.

Download **Boost 1.81** and extract it into `src/external/boost`. For example, you should have `config.hpp` in `src/external/boost/boost`.

If you want to just build the binaries, but don't care about Visual Studio solution files:
* (Release mode) Just run `compile.cmd`. The output will be in the folder `t2-output/win32-msvc-release-default`.
* (Debug mode) Just run `compile_debug.cmd`. The output will be in the folder`t2-output/win32-msvc-debug-default`.

If you want to build a release (i.e. have it save the binaries and put all the files needed to run the mod in a single folder):
* (Release mode) Just run `release.cmd`. The output will be in the folder `release`.
* (Debug mode) Just run `release_debug.cmd`. The output will be in the folder `debug`.

If you'd like to open the project in Visual Studio, run `sln-vs2022.cmd`, and then open `t2-output/aliasIsolation.sln`.
