#pragma once

struct SharedDllParams {
	char	aliasIsolationRootDir[2048] = {};
	char	cinematicToolsDllPath[2048] = {};
	bool	terminate = false;
	bool	cinematicToolsEnable = false;
};

SharedDllParams	getSharedDllParams();
void			setSharedDllParams(const SharedDllParams& params);
