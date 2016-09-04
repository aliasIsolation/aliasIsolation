# Overview

**Alias Isolation** is a mod for Alien: Isolation. It adds temporal anti-aliasing into the shipped game, and fixes a few small issues with the rendering.

The mod works by injecting itself into the executable, and hijacking D3D11 calls. It replaces a few shaders, and injects some of its own rendering.

# Usage

Save the mod files in a non-system directory. It could be *Program Files*, or *My Documents*, or anything else as long as no special permissions are required to access that direcotry. If you're using a pre-packaged release, please unzip it rather than launching the mod from within the zip.

The mod launches Alien: Isolation and hooks into it. The usage is slightly different depending on whether you have a Steam version of the game (see below). In order to function correctly, the mod also requires certain video settings in the game.

## If you have Alien: Isolation installed via Steam

Run **aliasIsolationInjector.exe**, then adjust video settings as described below.

If Steam is running, the mod will inject itself into it, and then launch the game. Then injection into Steam is necessary, as it is actually the Steam.exe process which launches the game. Even if AI.exe is started directly, it communicates with Steam, and then exits immediately, allowing Steam to launch it. In order to hook into the game, Alias Isolation hooks into Steam, and intercepts its CreateProcessW call, subsequently injecting itself into the child process. The only binaries that are injected into are Steam.exe and AI.exe.

In order to remove the hook from Steam, run **detachAll.cmd**.

## If you have a non-Steam version of Alien: Isolation

You need to point the injector to where the game is installed. This can be done by launching **aliasIsolationInjector.exe** with the install directory of the game as a parameter, for example:

`aliasIsolationInjector.exe "C:\games\Alien Isolation"`

Where "C:\games\Alien Isolation" is the directory containing AI.exe. No trailing backslash, please.

If no parameters are passed to the injector, it will read them from a file called **args.cfg**.


## Video Settings

Most importantly, **Anti-aliasing** must be set to **SMAA T1x**. The mod will not work at all if any other anti-aliasing mode is selected.

I have only been testing the mod with video settings maxed out. I don't guarantee any other settings will work correctly.

There's one notable exception: **Chromatic Aberration** needs to be **disabled**. This is necessary because the mod applies a sharpening filter to the rendered frame, and it needs to happen before Chromatic Aberration. The mod applies its own aberration effect afterwards (if you don't like it, edit the shader).

## Runtime toggle

To disable the mod at runtime, hit **Ctrl+Delete**. To re-enable it, hit **Ctrl+Insert**.


# Known issues

* No support for the unofficial Rift mode.
* Resource leaks upon resolution changes.
* RenderDoc, or any other software which also uses API hooking may break the rendering.
* SLI/CrossFire performance will probably be badly impacted, and depending on what the driver does, might also suffer from glitchy rendering.


# Building from source

You need to have Visual Studio 2015. Community Edition works fine.

If you want to just build the binaries, but don't care about Visual Studio solution files, just run **compile.cmd**. They output will be in **t2-output/win32-msvc-release-default**.

If you'd like to open the project in Visual Studio, run **sln-vs2015.cmd**, and run **t2-output/aliasIsolation.sln**.
