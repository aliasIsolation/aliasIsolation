local common = {
	Env = {
		GENERATE_PDB = "1",
		CPPDEFS = { 
			"_CRT_SECURE_NO_WARNINGS",
			"_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING",
			{ "_DEBUG"; Config = "*-*-debug-*"},
		},
		CCOPTS = { 
			"/FS",	-- fatal error C1041: cannot open program database 'filename.pdb'; if multiple CL.EXE write to the same .PDB file, please use /FS
			"/DEBUG",
			"/Zi",
			{ "/Od", "/MTd"; Config = "*-*-debug-*" },
			{ "/Od", "/MT"; Config = "*-*-release" },
		},
		CXXOPTS = { 
			"/std:c++17",
			"/EHsc",
			"/DEBUG",
			"/Zi",
			{ "/Od", "/MTd"; Config = "*-*-debug-*" },
			{ "/Od", "/MT"; Config = "*-*-release" },
		},
		PROGOPTS = {
			{"/NODEFAULTLIB:LIBCMT"; Config = "*-*-debug-*"}
		},
	}
}

Build {
	Units = "units.lua",
	Configs = {
		Config {
			Name = "win32-msvc",
			DefaultOnHost = "windows",
			Inherit = common,
			Tools = { "msvc-vs2019" } 
		},
	},
	IdeGenerationHints = {
		Msvc = {
			-- Remap config names to MSVC platform names (affects things like header scanning & debugging)
			PlatformMappings = {
				['win64-vs2019'] = 'x64',
				['win32-vs2019'] = 'x86',
			},
			-- Remap variant names to MSVC friendly names
			VariantMappings = {
				['release']    = 'Release',
				['debug']      = 'Debug',
				['production'] = 'Production',
			},
		},
    	MsvcSolutions = {
			['aliasIsolation.sln'] = { } -- will get everything
		},
	},
}
