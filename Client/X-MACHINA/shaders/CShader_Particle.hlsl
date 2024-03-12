
#define gkMaxParticleCount 1000

struct ParticleSystemIndex
{
	int Index;
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

struct ParticleInfo
{
	float3	WorldPos;
	float	CurTime;
	float3	WorldDir;
	float	LifeTime;
	int	    Alive;
	float2	StartEndScale;
	float   Padding;
};

StructuredBuffer<ParticleSystemInfo> gParticleSystem : register(t0, space0);
RWStructuredBuffer<ParticleInfo> gOutputParticles : register(u0, space1);
ConstantBuffer<ParticleSystemIndex> gIndex : register(b0);

[numthreads(1024, 1, 1)]
void CSParticle(int3 threadIndex : SV_DispatchThreadID)
{
    ParticleSystemInfo ps = gParticleSystem[gIndex.Index];
    gOutputParticles[threadIndex.x].LifeTime += ps.AccTime;
}