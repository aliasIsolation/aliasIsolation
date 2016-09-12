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

local nana = StaticLibrary {
	Name = "nana",
	Includes = { "src/external/nana/include" },
	Sources = {
		Glob { Dir = "src/external/nana/source", Extensions = {".cpp", ".h"} }
	}
}

local crashHandler = StaticLibrary
{
	Name = "crashHandler",
	Sources = {
		Glob { Dir = "src/crashHandler", Extensions = {".cpp", ".h"} },
	},
}

local dll = SharedLibrary {
	Name = "aliasIsolation",
	Depends = { minhook, crashHandler },
	Includes = {
		common_includes,
		"src/external/glm",
		"src/external/minhook/include",
		"src/external/stb",
		"src/external/d3dcompiler43",
	},
	Sources = {
		common_sources,
		Glob { Dir = "src/dll", Extensions = {".cpp", ".h", ".inl"} }
	},
	Libs = {
		 { "user32.lib", "src/external/d3dcompiler43/d3dcompiler.lib", "psapi.lib", "dbghelp.lib"; Config = {"win*"} },
	},
}

local injector = Program {
	Name = "aliasIsolationInjector",
	Includes = { common_includes },
	Sources = {
		common_sources,
		Glob { Dir = "src/injector", Extensions = {".cpp", ".h"} },
	},
	Libs = {
		 { "Shlwapi.lib", "user32.lib", "Advapi32.lib"; Config = {"win*"} },
	}
}

local injectorGui = Program {
	Name = "aliasIsolationInjectorGui",
	Depends = { nana, crashHandler },
	Includes = { common_includes, "src/external/nana/include" },
	Sources = {
		common_sources,
		Glob { Dir = "src/injectorGui", Extensions = {".cpp", ".h"} },
	},
	Libs = {
		 { "Shlwapi.lib", "user32.lib", "Advapi32.lib", "Comdlg32.lib", "Gdi32.lib", "Shell32.lib", "psapi.lib", "dbghelp.lib"; Config = {"win*"} },
	},
	Env = {
		PROGOPTS = {"/SUBSYSTEM:WINDOWS"}
	}
}

Default(dll)
Default(injector)
