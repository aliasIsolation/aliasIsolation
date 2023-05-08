local common = {
	Env = {
		GENERATE_PDB = "1",
		CPPDEFS = { 
			"WINVER=0x0601", -- We are only targeting Windows 7 and above.
			"_WIN32_WINNT=0x0601",
			{ "_DEBUG"; Config = "*-*-debug-*" },
			{ "NDEBUG"; Config = "*-*-release" },
			{ "ALIASISOLATION_ENABLE_PROFILER"; Config = "*-*-*-profile" },
			--{ "ALIASISOLATION_NO_TAA_PASS"; Config = "*-*-*-profile" },
			--{ "ALIASISOLATION_NO_CA_PASS"; Config = "*-*-*-profile" },
			--{ "ALIASISOLATION_NO_PS_OVERRIDES"; Config = "*-*-*-profile" },
			--{ "ALIASISOLATION_NO_VS_OVERRIDES"; Config = "*-*-*-profile" },
			--{ "ALIASISOLATION_NO_SMAA_VS"; Config = "*-*-*-profile" },
			--{ "ALIASISOLATION_NO_RGBM_VS"; Config = "*-*-*-profile" },
			--{ "ALIASISOLATION_NO_JITTER_ADD"; Config = "*-*-*-profile" },
			--{ "ALIASISOLATION_NO_JITTER_REMOVE"; Config = "*-*-*-profile" },
			--{ "ALIASISOLATION_NO_SMAA_JITTER_ADD"; Config = "*-*-*-profile" },
		},
		CCOPTS = { 
			"/arch:AVX",
			"/fp:fast",
			"/FS",	-- fatal error C1041: cannot open program database 'filename.pdb'; if multiple CL.EXE write to the same .PDB file, please use /FS
			"/DEBUG:FULL", -- Starting with VS2017, the default value for /DEBUG is FASTLINK, which affects debugging on machines where the build products don't exist.
			--"/Zi", -- Removes debugging information from the build products and stores this information in the PDB files instead, reducing overall binary file size.
			"/W4", -- Enables all 4 levels of compiler warnings, this helps discover potentially hard-to-find issues with the codebase.
			--"/FAcs", -- Output assembler.
			--"/FaASM/",
			"/GR-", -- No RTTI.
			"/GL", -- Whole Program Optimisation.
			{ "/Od", "/MTd"; Config = "*-*-debug-*" },
			{ "/O2", "/MT"; Config = "*-*-release" },
		},
		CXXOPTS = { 
			"/arch:AVX", -- AVX might be too new for some player's systems, if it is, we could drop down to SSE2.
			"/fp:fast",
			"/std:c++20",
			"/EHsc",
			"/DEBUG:FULL",
			--"/Zi",
			"/W4",
			--"/FAcs", -- Output assembler.
			--"/FaASM/",
			"/GR-", -- No RTTI.
			"/GL", -- Whole Program Optimisation.
			{ "/Od", "/MTd"; Config = "*-*-debug-*" },
			{ "/O2", "/MT"; Config = "*-*-release" },
		},
		PROGOPTS = {
			{"/NODEFAULTLIB:LIBCMT"; Config = "*-*-debug-*"},
		},
		SHLIBOPTS = {
			"/LTCG", -- Link Time Code Generation.
		}
	}
}

Build {
	Units = "units.lua",
	Configs = {
		Config {
			Name = "win64-msvc",
			SupportedHosts = { "windows" },
			Inherit = common,
			Tools = { { "msvc-vs2022"; TargetArch = "x64" } },
		},
		Config {
			Name = "win32-msvc",
			DefaultOnHost = "windows",
			Inherit = common,
			Tools = { { "msvc-vs2022"; TargetArch = "x86" } },
		},
	},
	Variants = {
		"release",
		"debug",
	},
	SubVariants = {
		"default",
		--"profile",
	},
	DefaultSubVariant = "default",
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
			},
		},
    	MsvcSolutions = {
			['aliasIsolation.sln'] = { } -- will get everything
		},
	},
}
