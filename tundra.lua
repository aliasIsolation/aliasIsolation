local common = {
	Env = {
		GENERATE_PDB = "1",
		CPPDEFS = { 
			"WINVER=0x0601", -- We are only targeting Windows 7 and above.
			"_WIN32_WINNT=0x0601", 
			"_CRT_SECURE_NO_WARNINGS",
			"_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING",
			{ "_DEBUG"; Config = "*-*-debug-*"},
		},
		CCOPTS = { 
			"/FS",	-- fatal error C1041: cannot open program database 'filename.pdb'; if multiple CL.EXE write to the same .PDB file, please use /FS
			"/DEBUG:FULL", -- Starting with VS2017, the default value for /DEBUG is FASTLINK, which affects debugging on machines where the build products don't exist.
			"/Zi", -- Removes debugging information from the build products and stores this information in the PDB files instead, reducing overall binary file size.
			{ "/Od", "/MTd"; Config = "*-*-debug-*" },
			{ "/Od", "/MT"; Config = "*-*-release" }, -- /O2 enables a certain subset of compiler optimisations that have an emphasis on execution speed, at the cost of debugging.
		},
		CXXOPTS = { 
			"/std:c++17",
			"/EHsc",
			"/DEBUG:FULL",
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
			Name = "win64-msvc",
			SupportedHosts = { "windows" },
			Inherit = common,
			Tools = { { "msvc-vs2019"; TargetArch = "x64" } },
		},
		Config {
			Name = "win32-msvc",
			DefaultOnHost = "windows",
			Inherit = common,
			Tools = { { "msvc-vs2019"; TargetArch = "x86" } }, 
		},
	},
	IdeGenerationHints = {
		Msvc = {
			-- Remap config names to MSVC platform names (affects things like header scanning & debugging)
			PlatformMappings = {
				['win64-msvc'] = 'x64',
				['win32-msvc'] = 'Win32',
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
