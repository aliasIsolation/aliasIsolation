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

#include "common.h"
#include "profiler.h"
#include <format>

using std::string;
using std::map;

#ifdef ALIASISOLATION_ENABLE_PROFILER
	#define PROFILER_ENABLE 1
#else
	#define PROFILER_ENABLE 0
#endif

// == Profiler ====================================================================================

#if PROFILER_ENABLE
// ImGui menu profiling variables.
extern std::string g_profilerStats;
#endif

Profiler Profiler::GlobalProfiler;

void Profiler::Initialize(ID3D11Device* profilerDevice, ID3D11DeviceContext* immContext)
{
    this->device = profilerDevice;
    this->context = immContext;
}

#if PROFILER_ENABLE

void Profiler::StartProfile(const string& name)
{
    ProfileData& profileData = profiles[name];
    _ASSERT(profileData.QueryStarted == FALSE);
    _ASSERT(profileData.QueryFinished == FALSE);
    
    if(profileData.DisjointQuery[currFrame] == NULL)
    {
        // Create the queries
        D3D11_QUERY_DESC desc;
        ZeroMemory(&desc, sizeof(D3D11_QUERY_DESC));
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        desc.MiscFlags = 0;
        DX_CHECK(device->CreateQuery(&desc, &profileData.DisjointQuery[currFrame]));

        desc.Query = D3D11_QUERY_TIMESTAMP;
        DX_CHECK(device->CreateQuery(&desc, &profileData.TimestampStartQuery[currFrame]));
        DX_CHECK(device->CreateQuery(&desc, &profileData.TimestampEndQuery[currFrame]));
    }

    // Start a disjoint query first
    context->Begin(profileData.DisjointQuery[currFrame]);

    // Insert the start timestamp    
    context->End(profileData.TimestampStartQuery[currFrame]);

    profileData.QueryStarted = TRUE;
}

void Profiler::EndProfile(const string& name)
{
    ProfileData& profileData = profiles[name];
    _ASSERT(profileData.QueryStarted == TRUE);
    _ASSERT(profileData.QueryFinished == FALSE);

    // Insert the end timestamp    
    context->End(profileData.TimestampEndQuery[currFrame]);

    // End the disjoint query
    context->End(profileData.DisjointQuery[currFrame]);

    profileData.QueryStarted = FALSE;
    profileData.QueryFinished = TRUE;
}

void Profiler::EndFrame()
{
    currFrame = (currFrame + 1) % QueryLatency;
    g_profilerStats.clear();

    // Iterate over all of the profiles
    ProfileMap::iterator iter;
    for(iter = profiles.begin(); iter != profiles.end(); iter++)
    {
        ProfileData& profile = (*iter).second;
        if(profile.QueryFinished == FALSE)
            continue;

        profile.QueryFinished = FALSE;

        if(profile.DisjointQuery[currFrame] == NULL)
            continue;

        // Get the query data
        UINT64 startTime = 0;
        while(context->GetData(profile.TimestampStartQuery[currFrame], &startTime, sizeof(startTime), 0) != S_OK);

        UINT64 endTime = 0;
        while(context->GetData(profile.TimestampEndQuery[currFrame], &endTime, sizeof(endTime), 0) != S_OK);

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
        while(context->GetData(profile.DisjointQuery[currFrame], &disjointData, sizeof(disjointData), 0) != S_OK);

        double time = 0.0;
        if (disjointData.Disjoint == FALSE) {
            UINT64 delta = endTime - startTime;
            double frequency = static_cast<double>(disjointData.Frequency);
            time = (delta / frequency) * 1000.0;
        }

		if (-1 != profile.lastValue) {
			const double blend = 0.1;
			time = time * blend + profile.lastValue * (1.0 - blend);
		}

		profile.lastValue = time;
        
        g_profilerStats += std::format("{}: {:2.2f}ms\n", (*iter).first, time);
    }
}

#else

void Profiler::StartProfile(const string& /*name*/)
{
}

void Profiler::EndProfile(const string& /*name*/)
{
}

void Profiler::EndFrame()
{
}

#endif

// == ProfileBlock ================================================================================

ProfileBlock::ProfileBlock(const std::string& name) : name(name)
{
    Profiler::GlobalProfiler.StartProfile(name);
}

ProfileBlock::~ProfileBlock()
{
    Profiler::GlobalProfiler.EndProfile(name);
}