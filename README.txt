Overview
--------

Alias Isolation is a mod for Alien: Isolation. It adds temporal anti-aliasing into the shipped game, and fixes a few small issues with the rendering.

The mod works by injecting itself into the executable, and hijacking D3D11 calls. It replaces a few shaders, and injects some of its own rendering.

Usage
-----

Save the mod files in a non-system directory. It could be "Program Files", or "My Documents", or anything else as long as no special permissions are required to access that direcotry. If you're using a pre-packaged release, please unzip it rather than launching the mod from within the zip.

Run "aliasIsolationInjectorGui.exe", and follow the displayed instructions. Once you press the "Launch Alien: Isolation" button, the mod launches Alien: Isolation and hooks into it. Exiting the app will remove the mod from the Alien. If the game is a Steam copy, the mod will inject itself into it Steam well, and then launch the game.

Injection into Steam is necessary, as it is actually the Steam.exe process which launches the game. Even if AI.exe is started directly, it communicates with Steam, and then exits immediately, allowing Steam to launch it. In order to hook into the game, Alias Isolation hooks into Steam, and intercepts its CreateProcessW call, subsequently injecting itself into the child process. The only binaries that are injected into are Steam.exe and AI.exe.

In order to remove the hook from Steam, run "detachAll.cmd".


Video Settings
--------------

Anti-aliasing must be set to SMAA T1x.
Chromatic Aberration must be disabled.
Motion blur must be enabled.


Runtime toggle
--------------

To disable the mod at runtime, hit "Ctrl+Delete". To re-enable it, hit "Ctrl+Insert".


Known issues
------------

* No support for the unofficial Rift mode.
* Crashes when used with recent versions of ReShade.
* RenderDoc, or any other software which also uses API hooking may break the rendering.
* SLI/CrossFire performance will probably be badly impacted, and depending on what the driver does, might also suffer from glitchy rendering.
* Some special effects can appear thinner or exhibit ghosting. Sparks are known to be eroded.


Building from source
--------------------

You need to have Visual Studio 2015. Community Edition works fine.

Download Boost 1.61 and put it into src/external/boost/boost. For example, src/external/boost/boost/config.hpp.
Compile Boost for static CRT, and put the libs into src/external/boost. For example, src/external/boost/libboost_chrono-vc140-mt-s-1_61.lib.

If you want to just build the binaries, but don't care about Visual Studio solution files, just run "compile.cmd". They output will be in "t2-output/win32-msvc-release-default".

If you'd like to open the project in Visual Studio, run "sln-vs2015.cmd", and then open "t2-output/aliasIsolation.sln".
