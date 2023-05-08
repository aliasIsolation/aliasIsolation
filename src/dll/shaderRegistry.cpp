#include "shaderRegistry.h"
#include "utilities.h"

#include <mutex>
#include <map>
#include <vector>
#include <cassert>

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>


extern ID3D11Device*	g_device;

namespace ShaderRegistry
{
	enum class ShaderType {
		Pixel,
		Vertex,
		Compute
	};

	struct ShaderReloadInfo {
		std::string	path;
		ShaderType	shaderType;
		FILETIME	lastModified;
	};

	union CompiledShader {
		ID3D11PixelShader*		ps;
		ID3D11VertexShader*		vs;
		ID3D11ComputeShader*	cs;
	};


	std::mutex							mutex;
	std::map<std::string, ShaderHandle>	psPathToHandle;
	std::map<std::string, ShaderHandle>	vsPathToHandle;
	std::map<std::string, ShaderHandle>	csPathToHandle;
	std::vector<ShaderReloadInfo>		shaderReloadInfo;
	std::vector<CompiledShader>			loadedShaders;
	std::vector<ID3D11DeviceChild*>		shadersToRelease;


	ShaderHandle addPixelShader(std::string path) {
		path = getDataFilePath("shaders\\" + path, true);

		mutex.lock();
		{
			auto existing = psPathToHandle.find(path);
			if (existing != psPathToHandle.end()) {
				mutex.unlock();
				return existing->second;
			}
		}
		ShaderHandle handle(shaderReloadInfo.size());
		shaderReloadInfo.push_back(ShaderReloadInfo { path, ShaderType::Pixel, FILETIME { 0, 0 } });
		loadedShaders.push_back(CompiledShader { nullptr });
		psPathToHandle[path] = handle;
		mutex.unlock();
		return handle;
	}

	ShaderHandle addVertexShader(std::string path) {
		path = getDataFilePath("shaders\\" + path, true);
		
		mutex.lock();
		{
			auto existing = vsPathToHandle.find(path);
			if (existing != vsPathToHandle.end()) {
				mutex.unlock();
				return existing->second;
			}
		}
		ShaderHandle handle(shaderReloadInfo.size());
		shaderReloadInfo.push_back(ShaderReloadInfo { path, ShaderType::Vertex, FILETIME { 0, 0 } });
		loadedShaders.push_back(CompiledShader { nullptr });
		vsPathToHandle[path] = handle;
		mutex.unlock();
		return handle;
	}

	ShaderHandle addComputeShader(std::string path) {
		path = getDataFilePath("shaders\\" + path, true);

		mutex.lock();
		{
			auto existing = csPathToHandle.find(path);
			if (existing != csPathToHandle.end()) {
				mutex.unlock();
				return existing->second;
			}
		}
		ShaderHandle handle(shaderReloadInfo.size());
		shaderReloadInfo.push_back(ShaderReloadInfo { path, ShaderType::Compute, FILETIME { 0, 0 } });
		loadedShaders.push_back(CompiledShader { nullptr });
		csPathToHandle[path] = handle;
		mutex.unlock();
		return handle;
	}

	bool reload(ShaderReloadInfo *const reloadInfo, CompiledShader *const shader) {
		LPCSTR profile =
			reloadInfo->shaderType == ShaderType::Pixel ? "ps_4_0" :
			reloadInfo->shaderType == ShaderType::Compute ? "cs_5_0" :
			"vs_4_0";

		std::vector<char> sourceBlob;
		for (int retry = 0; retry < 100; ++retry, Sleep(10)) {
			sourceBlob.clear();

            FILE* f;
			errno_t err = fopen_s(&f, reloadInfo->path.c_str(), "rb");
			if (err != 0) {
				continue;
			}

			fseek(f, 0, SEEK_END);
			long fileSize = ftell(f);
			fseek(f, 0, SEEK_SET);

			sourceBlob.resize(fileSize);
			const long rb = fread_s(sourceBlob.data(), sourceBlob.size(), 1, fileSize, f);

			fclose(f);
			if (rb == fileSize) {
				break;	// success
			}

			sourceBlob.clear();
		}

		if (0 == sourceBlob.size()) {
			return false;
		}

		ID3DBlob*	shaderBlob = nullptr;
		char*		shaderBinaryPtr = nullptr;
		size_t		shaderBinaryLength = 0;

		if (sourceBlob.size() > 4 && sourceBlob[0] == 'D' && sourceBlob[1] == 'X' && sourceBlob[2] == 'B' && sourceBlob[3] == 'C')
		{
			// Binary shader
			shaderBinaryPtr = sourceBlob.data();
			shaderBinaryLength = sourceBlob.size();
		}
		else
		{
			ID3DBlob* errorBlob = nullptr;

			const char* const entryPoint =	reloadInfo->shaderType == ShaderType::Pixel ? "mainPS" :
											reloadInfo->shaderType == ShaderType::Compute ? "mainCS" :
											"mainVS";

			HRESULT hr = D3DCompile(
				sourceBlob.data(), 
				sourceBlob.size(), 
				nullptr, 
				nullptr, 
				nullptr, 
				entryPoint, 
				profile, 
#ifdef _DEBUG
				// Do not optimise shaders during compilation when we are targeting debug.
				D3DCOMPILE_ENABLE_STRICTNESS,
#else
				// Otherwise, enable maximum optimisations for shaders during compilation.
				D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3,
#endif
				0, 
				&shaderBlob, 
				&errorBlob
			);

			if (FAILED(hr)) {
				if (errorBlob) {
					MessageBoxA(NULL, (char*)errorBlob->GetBufferPointer(), NULL, NULL);
					errorBlob->Release();
				}

				if (shaderBlob) {
				   shaderBlob->Release();
				}

				return false;
			}

			if (errorBlob) {
				errorBlob->Release();
			}

			shaderBinaryPtr		= (char*)shaderBlob->GetBufferPointer();
			shaderBinaryLength	= shaderBlob->GetBufferSize();
		}

		if (reloadInfo->shaderType == ShaderType::Pixel) {
			ID3D11PixelShader* ps = nullptr;
			g_device->CreatePixelShader(shaderBinaryPtr, shaderBinaryLength, nullptr, &ps);

			if (ps) {
				if (shader->ps) {
					shadersToRelease.push_back(shader->ps);
				}
				shader->ps = ps;
			}
		} else if (reloadInfo->shaderType == ShaderType::Vertex) {
			ID3D11VertexShader* vs = nullptr;
			g_device->CreateVertexShader(shaderBinaryPtr, shaderBinaryLength, nullptr, &vs);

			if (vs) {
				if (shader->vs) {
					shadersToRelease.push_back(shader->vs);
				}
				shader->vs = vs;
			}
		} else if (reloadInfo->shaderType == ShaderType::Compute) {
			ID3D11ComputeShader* cs = nullptr;
			g_device->CreateComputeShader(shaderBinaryPtr, shaderBinaryLength, nullptr, &cs);

			if (cs) {
				if (shader->cs) {
					shadersToRelease.push_back(shader->cs);
				}
				shader->cs = cs;
			}
		}

		if (shaderBlob) {
			shaderBlob->Release();
		}

		return true;
	}

	void reloadIfModified(ShaderReloadInfo *const reloadInfo, CompiledShader *const shader) {
		WIN32_FILE_ATTRIBUTE_DATA stat;
        ZeroMemory(&stat, sizeof(stat));
		if (GetFileAttributesExA(reloadInfo->path.c_str(), GetFileExInfoStandard, &stat)) {
			if (stat.ftLastWriteTime.dwHighDateTime != reloadInfo->lastModified.dwHighDateTime ||
				stat.ftLastWriteTime.dwLowDateTime  != reloadInfo->lastModified.dwLowDateTime)
			{
				if (reload(reloadInfo, shader)) {
					reloadInfo->lastModified.dwHighDateTime = stat.ftLastWriteTime.dwHighDateTime;
					reloadInfo->lastModified.dwLowDateTime  = stat.ftLastWriteTime.dwLowDateTime;
				}
			}
		}
	}

	void reloadModified() {
		mutex.lock();
		const size_t count = shaderReloadInfo.size();

		for (size_t i = 0; i < count; ++i) {
			reloadIfModified(&shaderReloadInfo[i], &loadedShaders[i]);
		}

		mutex.unlock();
	}

	ID3D11PixelShader* getPs(ShaderHandle h) {
		assert(shaderReloadInfo[h.id].shaderType == ShaderType::Pixel);
		return loadedShaders[h.id].ps;
	}

	ID3D11VertexShader* getVs(ShaderHandle h) {
		assert(shaderReloadInfo[h.id].shaderType == ShaderType::Vertex);
		return loadedShaders[h.id].vs;
	}

	ID3D11ComputeShader* getCs(ShaderHandle h) {
		assert(shaderReloadInfo[h.id].shaderType == ShaderType::Compute);
		return loadedShaders[h.id].cs;
	}


	void releaseUnused() {
		mutex.lock();
		for (ID3D11DeviceChild* o : shadersToRelease) {
			o->Release();
		}
		shadersToRelease.clear();
		mutex.unlock();
	}




	volatile bool	finishing = false;
	volatile bool	finished = true;
	HANDLE			shaderCompilerThreadHandle = nullptr;

	DWORD WINAPI shaderCompilerThread(void*)
	{
		while (!finishing) {
			reloadModified();
			Sleep(100);
		}

		finished = true;
		return 0;
	}

	void startCompilerThread()
	{
		finished = false;
		finishing = false;
		shaderCompilerThreadHandle = CreateThread(NULL, NULL, &shaderCompilerThread, NULL, NULL, NULL);
	}

	void stopCompilerThread()
	{
		finishing = true;
		WaitForSingleObject(shaderCompilerThreadHandle, 2000);
	}
}
