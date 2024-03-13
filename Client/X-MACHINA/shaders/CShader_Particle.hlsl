#include "Common.hlsl"

#define gkMaxParticleCount 1000

struct ParticleSystemIndexInfo
{
	int Index;
};

struct ParticleSharedInfo
{
    int AddCount;
};

struct ParticleSystemInfo
{
	int		AddCount;
	int		MaxCount;
	float	DeltaTime;
	float	AccTime;
	float	MinLifeTime;
	float	MaxLifeTime;
	float	MinSpeed;
	float	MaxSpeed;
	float	StartScale;
	float	EndScale;
	float2  Padding;
};

StructuredBuffer<ParticleSystemInfo> gParticleSystem : register(t0, space0);
RWStructuredBuffer<ParticleSharedInfo> gParticleShared : register(u0, space1);
RWStructuredBuffer<ParticleInfo> gOutputParticles : register(u0, space2);
ConstantBuffer<ParticleSystemIndexInfo> gIndex : register(b0);

[numthreads(1024, 1, 1)]
void CSParticle(int3 threadID : SV_DispatchThreadID)
{
    ParticleSystemInfo ps = gParticleSystem[gIndex.Index];

    if (threadID.x >= ps.MaxCount)
        return;

    gParticleShared[gIndex.Index].AddCount = ps.AddCount;
    GroupMemoryBarrierWithGroupSync();
	
    if (gOutputParticles[threadID.x].Alive == 0)
    {
        while (true)
        {
            int remaining = gParticleShared[gIndex.Index].AddCount;
			if (remaining <= 0)
                break;
			
            int expected = remaining;
            int desired = remaining - 1;
            int originalValue;
            InterlockedCompareExchange(gParticleShared[gIndex.Index].AddCount, expected, desired, originalValue);
			
            if (originalValue == expected)
            {
                gOutputParticles[threadID.x].Alive = 1;
                break;
            }
			
        }
		
        if (gOutputParticles[threadID.x].Alive == 1)
        {
            gOutputParticles[threadID.x].CurTime = 0.f;
            gOutputParticles[threadID.x].WorldPos = 0.f.xxx;
        }
    }
    else
    {
        gOutputParticles[threadID.x].CurTime += ps.DeltaTime;
        if (gOutputParticles[threadID.x].LifeTime < gOutputParticles[threadID.x].CurTime)
        {
            gOutputParticles[threadID.x].Alive = 0;
            return;
        }
        
        float t = gOutputParticles[threadID.x].CurTime / gOutputParticles[threadID.x].LifeTime;
        float3 v = float3(0.f, 10.f, 0.f);
        gOutputParticles[threadID.x].WorldPos += dir
    }
	
}