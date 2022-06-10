require "tundra.syntax.glob"
require "tundra.syntax.files"

local common_includes = {
	"src/common",
	"src/crashHandler"
}

local common_sources = {
	Glob { Dir = "src/common", Extensions = {".cpp" } }
}

local minhook = StaticLibrary {
	Name = "minhook",
	Sources = {
		Glob { Dir = "src/external/minhook/src", Extensions = {".c", ".h"} }
	}
}

local imgui = StaticLibrary {
	Name = "imgui",
	Includes = { "src/external/imgui" },
	Sources = {
		Glob { Dir = "src/external/imgui", Extensions = {".cpp", ".h"} }
	}
}

local crashHandler = StaticLibrary {
	Name = "crashHandler",
	Sources = {
		Glob { Dir = "src/crashHandler", Extensions = {".cpp", ".h"} },
	}
}

local dll = SharedLibrary {
	Name = "aliasIsolation",
	Depends = { minhook, crashHandler, imgui },
	Includes = {
		common_includes,
		"src/external/glm",
		"src/external/minhook/include",
		"src/external/stb",
		"src/external/imgui",
	},
	Sources = {
		common_sources,
		Glob { Dir = "src/dll", Extensions = {".cpp", ".h", ".inl"} }
	},
	Libs = {
		 { "user32.lib", "d3dcompiler.lib", "d3d11.lib", "psapi.lib", "dbghelp.lib"; Config = {"win*"} },
	},
}

local cinematicTools = SharedLibrary {
	Name = "cinematicTools",
	Depends = { minhook },
	Includes = {
		"src/external/DirectXTK/Inc",
		"src/external/minhook/include",
		"src/external/boost",
		"src/cinematicTools",
		"src/cinematicTools/renderer",
		"src/external/FW1FontWrapper_1_1",
		"src/external/FX11/inc",
	},
	Sources = {
		Glob { Dir = "src/cinematicTools", Extensions = {".cpp", ".h"} },
		Glob { Dir = "src/external/FW1FontWrapper_1_1/Source", Extensions = {".cpp", ".h"} },
	},
	Libs = {
		{
			"Shlwapi.lib", "user32.lib", "Advapi32.lib", "Comdlg32.lib", "Gdi32.lib", "Shell32.lib", "psapi.lib", "dbghelp.lib", "XInput.lib";
			Config = {"win*"}
		},
		{
			"src/external/boost/stage/lib/libboost_chrono-vc142-mt-sgd-x32-1_79.lib",
			"src/external/boost/stage/lib/libboost_system-vc142-mt-sgd-x32-1_79.lib",
			"src/external/boost/stage/lib/libboost_date_time-vc142-mt-sgd-x32-1_79.lib",
			"src/external/FX11/Bin/Desktop_2019_Win10/Win32/Debug/Effects11d.lib",
			"src/external/DirectXTK/Bin/Desktop_2019/Win32/Debug/DirectXTK.lib";
			Config = {"win32-*-debug"}
		},
		{
			"src/external/boost/stage/lib/libboost_chrono-vc142-mt-s-x32-1_79.lib",
			"src/external/boost/stage/lib/libboost_system-vc142-mt-s-x32-1_79.lib",
			"src/external/boost/stage/lib/libboost_date_time-vc142-mt-s-x32-1_79.lib",
			"src/external/FX11/Bin/Desktop_2019_Win10/Win32/Release/Effects11.lib",
			"src/external/DirectXTK/Bin/Desktop_2019/Win32/Release/DirectXTK.lib";
			Config = {"win32-*-release"}
		},
		{
			"src/external/boost/stage/lib/libboost_chrono-vc142-mt-sgd-x64-1_79.lib",
			"src/external/boost/stage/lib/libboost_system-vc142-mt-sgd-x64-1_79.lib",
			"src/external/boost/stage/lib/libboost_date_time-vc142-mt-sgd-x64-1_79.lib",
			"src/external/FX11/Bin/Desktop_2019_Win10/x64/Debug/Effects11d.lib",
			"src/external/DirectXTK/Bin/Desktop_2019/x64/Debug/DirectXTK.lib";
			Config = {"win64-*-debug"}
		},
		{
			"src/external/boost/stage/lib/libboost_chrono-vc142-mt-s-x64-1_79.lib",
			"src/external/boost/stage/lib/libboost_system-vc142-mt-s-x64-1_79.lib",
			"src/external/boost/stage/lib/libboost_date_time-vc142-mt-s-x64-1_79.lib",
			"src/external/FX11/Bin/Desktop_2019_Win10/x64/Release/Effects11.lib",
			"src/external/DirectXTK/Bin/Desktop_2019/x64/Release/DirectXTK.lib";
			Config = {"win64-*-release"}
		}
	}
}

Default(dll)
Default(injector)
Default(injectorGui)
Default(cinematicTools)
