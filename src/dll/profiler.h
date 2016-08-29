// Based on MJP's code, and then hacked and chopped a bit:

//=================================================================================================
//
//	Query Profiling Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

#include <d3d11.h>
#include <string>
#include <map>

class Profiler
{

public:

    static Profiler GlobalProfiler;

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* immContext);

    void StartProfile(const std::string& name);
    void EndProfile(const std::string& name);

    void EndFrame();

protected:

    // Constants
    static const UINT64 QueryLatency = 9;

    struct ProfileData
    {
        ID3D11Query* DisjointQuery[QueryLatency];
        ID3D11Query* TimestampStartQuery[QueryLatency];
        ID3D11Query* TimestampEndQuery[QueryLatency];
        BOOL QueryStarted;
        BOOL QueryFinished;
		double lastValue;

        ProfileData() : QueryStarted(FALSE), QueryFinished(FALSE), lastValue(-1) {
			for (UINT64 i = 0; i < QueryLatency; ++i) {
				DisjointQuery[i] = nullptr;
				TimestampStartQuery[i] = nullptr;
				TimestampEndQuery[i] = nullptr;
			}
		}
    };

    typedef std::map<std::string, ProfileData> ProfileMap;

    ProfileMap profiles;
    UINT64 currFrame;

    ID3D11Device*			device;
    ID3D11DeviceContext*	context;
};

class ProfileBlock
{
public:

    ProfileBlock(const std::string& name);
    ~ProfileBlock();

protected:

    std::string name;
};
