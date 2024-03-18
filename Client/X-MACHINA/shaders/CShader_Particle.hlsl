#include "Common.hlsl"

#define gkMaxParticleCount 1000

#define gkSimulationSpace_Local 0
#define gkSimulationSpace_World 1

#define PSColor_Color                       0
#define PSColor_Gradient                    1
#define PSColor_RandomBetweenTwoColors      2
#define PSColor_RandomBetweenTwoGradient    3

struct ParticleSystemIndexInfo
{
	int Index;
};

struct ParticleSharedInfo
{
    int AddCount;
};

struct PSColor
{
    float4 FirstColor;
	float4 SecondColor;
	float4 FirstGradient;
	float4 SecondGradient;
	uint   ColorOption;
	float3 Padding;
};

struct ColorOverLifetime {
	uint    IsOn;
	float3  Padding;
	float4  StartColor;
    float4  EndColor;
};

struct ParticleSystemInfo
{
    float3  WorldPos;
    int	    TextureIndex;
	float4  Color;
    
	int		AddCount;
	int		MaxParticles;
	float	DeltaTime;
	float	TotalTime;
    
	float2	StartLifeTime;
	float2	StartSpeed;
	float4	StartSize3D;
	float4	StartRotation3D;
	float2	StartSize;
	float2	StartRotation;
    PSColor StartColor;
	float	GravityModifier;
    uint    SimulationSpace;
    float   SimulationSpeed;
    int     SizeOverLifeTime;
    ColorOverLifetime ColorOverLifetime;
};

float4 ComputeColor(PSColor color, float t, float r)
{
    float4 result = float4(1.f, 1.f, 1.f, 1.f);
    switch (color.ColorOption)
    {
        case PSColor_Color:
            result = color.FirstColor;
            break;
        case PSColor_Gradient:
            result = lerp(color.FirstColor, color.FirstGradient, frac(t / 5));
            break;
        case PSColor_RandomBetweenTwoColors:
            result = lerp(color.FirstColor, color.SecondColor, r);
            break;
        case PSColor_RandomBetweenTwoGradient:
            float4 firstColor = lerp(color.FirstColor, color.FirstGradient, frac(t / 2));
            float4 secondColor = lerp(color.SecondColor, color.SecondGradient, frac(t / 2));
            result = lerp(firstColor, secondColor, r);
            break;
    }
    return result;
}

StructuredBuffer<ParticleSystemInfo> gParticleSystem : register(t0, space0);
RWStructuredBuffer<ParticleSharedInfo> gParticleShared : register(u0, space1);
RWStructuredBuffer<ParticleInfo> gOutputParticles : register(u0, space2);
ConstantBuffer<ParticleSystemIndexInfo> gIndex : register(b0);

[numthreads(1024, 1, 1)]
void CSParticle(int3 threadID : SV_DispatchThreadID)
{
    ParticleSystemInfo ps = gParticleSystem[gIndex.Index];
    
    // ���� �õ� ����
    NumberGenerator rand;
    rand.SetSeed(threadID.x);
    
    // �ִ� ��ƼŬ�� ��� ��� ����
    if (threadID.x >= ps.MaxParticles)
        return;
    
    // ��ƼŬ ���� ����
    gParticleShared[gIndex.Index].AddCount = ps.AddCount;
    GroupMemoryBarrierWithGroupSync();
	
    // ��ƼŬ�� ������ ��쿡�� ����
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
		
        // ��ƼŬ �ʱ�ȭ
        if (gOutputParticles[threadID.x].Alive == 1)
        {
            // Start �ʱ�ȭ
            gOutputParticles[threadID.x].TextureIndex = ps.TextureIndex;
            gOutputParticles[threadID.x].CurTime = 0.f;
            gOutputParticles[threadID.x].LocalPos = (float3)0.f;
            gOutputParticles[threadID.x].StartPos = ps.WorldPos;
            gOutputParticles[threadID.x].WorldPos = ps.WorldPos;
            gOutputParticles[threadID.x].WorldDir = rand.GetRandomFloat3(-1.f, 1.f);
            gOutputParticles[threadID.x].LifeTime = rand.GetRandomFloat(ps.StartLifeTime.x, ps.StartLifeTime.y);
            gOutputParticles[threadID.x].StartSpeed = rand.GetRandomFloat(ps.StartSpeed.x, ps.StartSpeed.y);
            gOutputParticles[threadID.x].StartSize = (ps.StartSize3D.w == 0.f) ? rand.GetRandomFloat(ps.StartSize.x, ps.StartSize.y) : float2(ps.StartSize3D.x, ps.StartSize3D.y);
            gOutputParticles[threadID.x].StartRotation = (ps.StartRotation3D.w == 0.f) ? float3(0.f, rand.GetRandomFloat(ps.StartRotation.x, ps.StartRotation.y), 0.f) : ps.StartRotation3D.xyz;
            gOutputParticles[threadID.x].StartColor = ComputeColor(ps.StartColor, ps.TotalTime, rand.GetCurrentFloat());

            // Final �ʱ�ȭ
            gOutputParticles[threadID.x].FinalSize = gOutputParticles[threadID.x].StartSize;
            gOutputParticles[threadID.x].FinalColor = gOutputParticles[threadID.x].StartColor;
        }
    }
    if (gOutputParticles[threadID.x].Alive == 1)
    {
        // ��ƼŬ ���� �ð� ���
        gOutputParticles[threadID.x].CurTime += ps.DeltaTime;
        
        // ��ƼŬ ���� �ð��� ������ ũ�� ����
        if (gOutputParticles[threadID.x].LifeTime < gOutputParticles[threadID.x].CurTime)
        {
            gOutputParticles[threadID.x].Alive = 0;
            return;
        }
        
        // ��ƼŬ ���� �ð��� 0~1�� ����ȭ
        float lifeRatio = gOutputParticles[threadID.x].CurTime / gOutputParticles[threadID.x].LifeTime;

        // SizeOverLifeTime�� ���� �ִٸ� �ð��� ���� ������ ����
        if (ps.SizeOverLifeTime == true)
        {
            gOutputParticles[threadID.x].FinalSize = gOutputParticles[threadID.x].StartSize * lifeRatio;
        }
        
        // ColorOverLifeTime�� ���� �ִٸ� �ð��� ���� StartColor���� EndColor�� ��ȭ
        if (ps.ColorOverLifetime.IsOn == true)
        {
            gOutputParticles[threadID.x].FinalColor = lerp(gOutputParticles[threadID.x].StartColor * ps.ColorOverLifetime.StartColor, gOutputParticles[threadID.x].StartColor * ps.ColorOverLifetime.EndColor, lifeRatio);
        }
        
        // �ӵ��� �߷� ���
        float speed = gOutputParticles[threadID.x].StartSpeed * ps.DeltaTime;
        float3 gravity = float3(0.f, -9.81f * 0.01f * ps.GravityModifier, 0.f) * pow(lifeRatio, 2) * ps.SimulationSpeed;
        
        // �ӵ��� �߷��� ��ƼŬ ���� �����ǿ� ����
        gOutputParticles[threadID.x].LocalPos += normalize(gOutputParticles[threadID.x].WorldDir) * speed + gravity;
        
        // ���� �������� ��ġ ����
        float3 prevPos = gOutputParticles[threadID.x].WorldPos;
        
        // SimulationSpace�� Local�� ��� �����Ӹ��� ���� �ִ� ������Ʈ ��ġ ����, World�� ��� �����ÿ��� ����
        if (ps.SimulationSpace == gkSimulationSpace_Local)
            gOutputParticles[threadID.x].WorldPos = gOutputParticles[threadID.x].LocalPos + ps.WorldPos;
        else if (ps.SimulationSpace == gkSimulationSpace_World)
            gOutputParticles[threadID.x].WorldPos = gOutputParticles[threadID.x].LocalPos + gOutputParticles[threadID.x].StartPos;

        // ���� �����Ӱ� ���� �����ӿ� ���� �̵� ���� ����
        gOutputParticles[threadID.x].MoveDir = normalize(gOutputParticles[threadID.x].WorldPos - prevPos);
    }
	
}