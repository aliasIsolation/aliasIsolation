require "tundra.syntax.glob"
require "tundra.syntax.files"

local common_includes = {
	"src/common"
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

local dll = SharedLibrary {
	Name = "aliasIsolation",
	Depends = { minhook },
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
		 { "user32.lib", "src/external/d3dcompiler43/d3dcompiler.lib"; Config = {"win*"} },
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

Default(dll)
Default(injector)
