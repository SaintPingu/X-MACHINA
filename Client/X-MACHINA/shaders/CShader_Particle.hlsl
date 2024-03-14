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
    float3  WorldPos;
	int		AddCount;
	int		MaxCount;
	float	DeltaTime;
	float	AccTime;
	float	MinLifeTime;
	float	MaxLifeTime;
	float	MinSpeed;
	float	MaxSpeed;
	float	StartScale;
	float4  Color;
	float	EndScale;
    int	    TextureIndex;
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
    
    // particle color
    gOutputParticles[threadID.x].TextureIndex = ps.TextureIndex;
    gOutputParticles[threadID.x].Color = ps.Color;

    // particle shared 
    gParticleShared[gIndex.Index].AddCount = ps.AddCount;
    GroupMemoryBarrierWithGroupSync();
	
    // particle logic
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
            gOutputParticles[threadID.x].StartEndScale = float2(ps.StartScale, ps.EndScale);
            gOutputParticles[threadID.x].WorldPos = gOutputParticles[threadID.x].LocalPos + ps.WorldPos;
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
        
        float lifeRatio = gOutputParticles[threadID.x].CurTime / gOutputParticles[threadID.x].LifeTime;
        
        gOutputParticles[threadID.x].Color.a = 1 - lifeRatio;
        gOutputParticles[threadID.x].WorldPos += normalize(gOutputParticles[threadID.x].WorldDir) * ps.DeltaTime;
    }
	
}