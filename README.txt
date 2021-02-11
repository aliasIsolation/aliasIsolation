Overview
--------

Alias Isolation is a mod for Alien: Isolation. It adds temporal anti-aliasing into the shipped game, and fixes a few small issues with the rendering.

The mod works by injecting itself into the executable, and hijacking D3D11 calls. It replaces a few shaders, and injects some of its own rendering.

This fork aims to maintain the mod, resolve existing issues, add new functionality and port the mod to new platforms.

Suggestions for new features or pull requests for this mod are welcome!


Planned features
----------------

New injection method:
The current injection method relies on injecting into the game store's executable and then injecting into Alien: Isolation's executable (AI.exe) after it is launched, this has proven to be troublesome/unreliable for a number of users (based on the issues open on GitHub) and will not work under Proton.
I am working on a new method of injection that does not require the mod to be injected into the game store's executable (therefore allowing support for Proton), however some work on the UI side of things is required, to replicate the settings editor the injector GUI provided.
Storing the mod's data in the same directory as the AI.exe executable may be required for this new method.
I intend to maintain compatibility with the current injection method as well, for users that prefer the functionality that it provides.
I will post a pre-release (beta) build with support for this feature soon!
This is in active development.

Linux (Proton) support:
Basic support for Proton requires little work and the anti-aliasing portion of the mod is already working under it after a bit of tweaking.
11/02/2021: I have noted a very considerable frame drop with the mod's TAA hooks active (under an NVIDIA GPU with proprietary drivers, I have not yet tested open-source drivers), some more testing is needed, particularly with AMD GPU users, to try to narrow down the issue.
I will post a pre-release (beta) build with support for this feature soon!
This is in active development.

VR mode (MotherVR) support:
I will wait for the source code release of MotherVR before investigating this any further as I want to ensure I avoid making any changes that would conflict with MotherVR.
This is planned, but I have not yet started development towards this.

Windows (UWP/Windows Store) support:
The UWP API imposes restrictions on what applications can do and has higher security than the regular Win32 API.
It seems Lab42 built the game for 64-bit/x64 machines (as opposed to the Steam and EGS builds, which are all 32-bit/x86), which may cause issues with mods like Alias Isolation, I have not yet verified this.
This is planned, but I have not yet started development towards this.

Linux (Native) support:
On the surface it seems that it would take a considerable amount of work to port Alias Isolation over to Linux and to make it work with the Linux native port by Feral, I am exploring the possibility of this.
This is planned, but I have not yet started development towards this.


Usage
-----

Save the mod files in a non-system directory. It could be "Program Files", or "My Documents", or anything else as long as no special permissions are required to access that directory. If you're using a pre-packaged release, please unzip it rather than launching the mod from within the zip.

Run "aliasIsolationInjectorGui.exe", and follow the displayed instructions. Once you press the "Launch Alien: Isolation" button, the mod launches Alien: Isolation and hooks into it. Exiting the app will remove the mod from the Alien. If the game is a Steam copy, first exit Steam before running the injector. If Steam is running before the injector is launched, it will not work. The mod will inject itself into Steam as well, and then launch the game.

Injection into Steam is necessary, as it is actually the Steam.exe process which launches the game. Even if AI.exe is started directly, it communicates with Steam, and then exits immediately, allowing Steam to launch it. In order to hook into the game, Alias Isolation hooks into Steam, and intercepts its CreateProcessW call, subsequently injecting itself into the child process. The only binaries that are injected into are Steam.exe and AI.exe.

In order to remove the hook from Steam, run "detachAll.cmd".


Video settings
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

You need to have Visual Studio 2015 or 2019. Community Edition works fine.

Download Boost 1.61 and put it into src/external/boost/boost. For example, src/external/boost/boost/config.hpp.
Compile Boost for static CRT, and put the libs into src/external/boost. For example, src/external/boost/libboost_chrono-vc140-mt-s-1_61.lib.

If you want to just build the binaries, but don't care about Visual Studio solution files, just run "compile.cmd". They output will be in "t2-output/win32-msvc-release-default".

If you'd like to open the project in Visual Studio, run "sln-vs2015.cmd", and then open "t2-output/aliasIsolation.sln".
