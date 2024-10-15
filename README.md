## Overview
Alias Isolation is a mod for Alien: Isolation. It adds temporal anti-aliasing into the shipped game, and fixes a few small issues with the rendering.

The mod works by injecting itself into the executable, and hijacking D3D11 calls. It replaces a few shaders, and injects some of its own rendering.

Suggestions for new features or pull requests for this mod are welcome!

## Installation instructions
Extract the following files from the `AliasIsolation-<version>.7z` archive, obtainable via the releases page, to the folder where _Alien: Isolation_ is installed (you should be in a folder that has a file called `AI.exe`):
1. Extract the `mods` folder to your _Alien: Isolation_ folder.
2. Extract `aliasIsolation.asi` and `d3d11.dll` to your _Alien: Isolation_ folder.

> [!NOTE]
> If you're using Linux and want to play the game via an older version of Proton (version < Proton 8), you _might_ need to perform an extra step to let Proton know that you want it to use the ASI loader:
> * Add this launch option to the game, in Steam: `WINEDLLOVERRIDES="d3d11=n,b"`

## Video settings
These options must be set to the correct values for the mod to work properly.

| Option | Value |
| --- | --- |
| Anti-aliasing | **SMAA T1x** |
| Chromatic Aberration | **Disabled** |
| Motion Blur | **Enabled** |

## Runtime settings
To disable the mod at runtime, hit `Ctrl+Delete`. To re-enable it, hit `Ctrl+Insert`.
To show/hide the mod menu at runtime, hit `DELETE`.

## Known issues
* No support for the _MotherVR_ mod.
* RenderDoc, Windows Auto HDR, MSI Afterburner, Rivatuner, or any other software which also uses API hooking may break the rendering.
* Some particle effects can appear thinner or exhibit ghosting. Sparks and embers are known to be eroded.
* Signs with text can noticeably "fade in" when the player's camera is stationary.
* While the motion tracker is in use, the _Depth of Field_ shader seems to cause the tracker's LED highlights to appear along the edges of the screen.
* The _Epic Games Store_ release of the game may crash with a fatal error at `aliasIsolation_hookableOverlayRender`.

## Building from source
1. Clone this repo using Git.
2. Install CMake, Visual Studio 2022 (Community edition is enough) and choose the C++ packages (including all the Clang support packages) in the installer.
3. Configure the project using CMake's GUI to use Visual Studio 2022, an x86 target (very important!) and `ClangCL` as the toolset parameter.
5. Modify `tracy`'s `CMakeLists.txt` to force `TRACY_ENABLE` to `OFF` - your game will run out of memory from the performance profiler otherwise.
6. Go to the output directory you chose using CMake, open the solution (`.sln`) in Visual Studio and build for `RelWithDebInfo`.
7. The compiled shaders (`.cso`) will be in the output directory, collect these files to create the below structure (have a look at a recent release to see how they're packaged if needs be).

* `mods`
  * `aliasIsolation`
      * `data`
          * `shaders`
              * `.cso` files
          * `textures`
              * `aliasIsolationLogo.png`
