require "tundra.syntax.glob"
require "tundra.syntax.files"

local common_includes = {
	"src/common",
	"src/crashHandler"
}

local common_sources = {
	Glob { Dir = "src/common", Extensions = { ".cpp" } }
}

local minhook = StaticLibrary {
	Name = "minhook",
	Sources = {
		Glob { Dir = "src/external/minhook/src", Extensions = { ".c", ".h" } }
	}
}

local imgui = StaticLibrary {
	Name = "imgui",
	Includes = { "src/external/imgui" },
	Sources = {
	    "src/external/imgui/backends/imgui_impl_win32.cpp",
	    "src/external/imgui/backends/imgui_impl_win32.h",
	    "src/external/imgui/backends/imgui_impl_dx11.cpp",
	    "src/external/imgui/backends/imgui_impl_dx11.h",
	    Glob {
	        Dir = "src/external/imgui",
	        Extensions = { ".cpp", ".h" },
	        Recursive = false
	    }
	}
}

local crashHandler = StaticLibrary {
	Name = "crashHandler",
	Sources = {
		Glob { Dir = "src/crashHandler", Extensions = { ".cpp", ".h" } },
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
		Glob { Dir = "src/dll", Extensions = { ".cpp", ".h", ".inl" } }
	},
	Libs = {
		 { "user32.lib", "d3dcompiler.lib", "d3d11.lib", "psapi.lib", "dbghelp.lib"; Config = { "win*" } },
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
		"src/external/FW1FontWrapper/FW1FontWrapper/Source",
		"src/external/FX11/inc",
	},
	Sources = {
		Glob { Dir = "src/cinematicTools", Extensions = { ".cpp", ".h" } },
		Glob { Dir = "src/external/FW1FontWrapper/FW1FontWrapper/Source", Extensions = { ".cpp", ".h" } },
	},
	Libs = {
		{
			"Shlwapi.lib", "user32.lib", "Advapi32.lib", "Comdlg32.lib", "Gdi32.lib", "Shell32.lib", "psapi.lib", "dbghelp.lib", "XInput.lib";
			Config = { "win*" }
		},
		{
			"src/external/boost/stage/lib/libboost_chrono-vc143-mt-sgd-x32-1_81.lib",
			"src/external/boost/stage/lib/libboost_system-vc143-mt-sgd-x32-1_81.lib",
			"src/external/boost/stage/lib/libboost_date_time-vc143-mt-sgd-x32-1_81.lib",
			"src/external/FX11/Bin/Desktop_2022_Win10/Win32/Debug/Effects11d.lib",
			"src/external/DirectXTK/Bin/Desktop_2022/Win32/Debug/DirectXTK.lib";
			Config = { "win32-*-debug" }
		},
		{
			"src/external/boost/stage/lib/libboost_chrono-vc143-mt-s-x32-1_81.lib",
			"src/external/boost/stage/lib/libboost_system-vc143-mt-s-x32-1_81.lib",
			"src/external/boost/stage/lib/libboost_date_time-vc143-mt-s-x32-1_81.lib",
			"src/external/FX11/Bin/Desktop_2022_Win10/Win32/Release/Effects11.lib",
			"src/external/DirectXTK/Bin/Desktop_2022/Win32/Release/DirectXTK.lib";
			Config = { "win32-*-release" }
		},
		{
			"src/external/boost/stage/lib/libboost_chrono-vc143-mt-sgd-x64-1_81.lib",
			"src/external/boost/stage/lib/libboost_system-vc143-mt-sgd-x64-1_81.lib",
			"src/external/boost/stage/lib/libboost_date_time-vc143-mt-sgd-x64-1_81.lib",
			"src/external/FX11/Bin/Desktop_2022_Win10/x64/Debug/Effects11d.lib",
			"src/external/DirectXTK/Bin/Desktop_2022/x64/Debug/DirectXTK.lib";
			Config = { "win64-*-debug" }
		},
		{
			"src/external/boost/stage/lib/libboost_chrono-vc143-mt-s-x64-1_81.lib",
			"src/external/boost/stage/lib/libboost_system-vc143-mt-s-x64-1_81.lib",
			"src/external/boost/stage/lib/libboost_date_time-vc143-mt-s-x64-1_81.lib",
			"src/external/FX11/Bin/Desktop_2022_Win10/x64/Release/Effects11.lib",
			"src/external/DirectXTK/Bin/Desktop_2022/x64/Release/DirectXTK.lib";
			Config = { "win64-*-release" }
		}
	}
}

Default(dll)
--Default(cinematicTools)
