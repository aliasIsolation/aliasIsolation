#pragma once

struct SharedDllParams {
	char	aliasIsolationRootDir[512];
	bool	terminate;
};

SharedDllParams	getSharedDllParams();
void			setSharedDllParams(const SharedDllParams& params);
