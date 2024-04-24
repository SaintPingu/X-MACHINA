#include "Common.hlsl"

#define EPSILON 0.0001

#define gkSimulationSpace_Local 0
#define gkSimulationSpace_World 1

#define PSValue_Constant                    0
#define PSValue_RandomBetweenTwoConstants   1

#define PSColor_Color                       0
#define PSColor_Gradient                    1
#define PSColor_RandomBetweenTwoColors      2
#define PSColor_RandomBetweenTwoGradient    3

#define PSShape_None        0
#define PSShape_Sphere      1
#define PSShape_HemiSphere  2
#define PSShape_Cone        3
#define PSShape_Box         4

#define gkCOL_GradientCount  4

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
    float4  FirstColor;
	float4  SecondColor;
	float4  FirstGradient;
	float4  SecondGradient;
	uint    ColorOption;
    float3  Padding;
};

struct PSShape
{
    uint    ShapeType;
	float3	Padding;
    float   Angle;
    float   Radius;
    float   RadiusThickness;
    float   Arc;
};

struct VelocityOverLifetime 
{
	uint    IsOn;
    float3  Linear;
    float   SpeedModifier;
    float3  Padding;
};

struct ColorOverLifetime 
{
	uint    IsOn;
	uint    Count;
	float2  Padding;
	float   Times[gkCOL_GradientCount];
    float4  Colors[gkCOL_GradientCount];
};

struct RotationOverLifetime 
{
	uint    IsOn;
	float   FirstAngularVelocity;
	float   SecondAngularVelocity;
    uint    ValueOption;
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
    int     SizeOverLifetime;
    
	VelocityOverLifetime	VelocityOverLifetime;
    ColorOverLifetime       ColorOverLifetime;
    RotationOverLifetime    RotationOverLifetime;
    PSShape Shape;
};

float4 SetStartColor(PSColor color, float t, float r)
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

void SetSphereStartPosDir(PSShape shape, inout NumberGenerator rand, inout float3 pos, inout float3 dir)
{
    // 파티클 초기 위치 설정
    float sphereCenter          = float3(0.f, 0.f, 0.f);
    float sphereRadius          = shape.Radius;
    float sphereRadiusThickness = shape.RadiusThickness;
    
    float theta = 2.f * 3.141592 * rand.GetCurrentFloat() / (shape.ShapeType == PSShape_HemiSphere ? 2.f : 1.f);
    float phi = acos(2.f * rand.GetCurrentFloat() - 1.f);
    float sinPhi = sin(phi);
    
    float r = rand.GetRandomFloat(1 - sphereRadiusThickness, sphereRadius);
    
    float x = r * sinPhi * cos(theta);
    float y = r * sinPhi * sin(theta);
    float z = r * cos(phi);
    
    pos = sphereCenter + float3(x, y, z) * sphereRadius;
    
    // 파티클 초기 방향 설정
    dir = normalize(pos - sphereCenter);
}

void SetConeStartPosDir(PSShape shape, inout NumberGenerator rand, inout float3 pos, inout float3 dir)
{
    
}

void SetBoxStartPosDir(PSShape shape, inout NumberGenerator rand, inout float3 pos, inout float3 dir)
{
    
}

void SetStartPosDir(PSShape shape, inout NumberGenerator rand, inout float3 pos, inout float3 dir)
{
    switch (shape.ShapeType)
    {
        case PSShape_Sphere:
        case PSShape_HemiSphere:
            SetSphereStartPosDir(shape, rand, pos, dir);
            break;
        case PSShape_Cone:
            SetConeStartPosDir(shape, rand, pos, dir);
            break;
        case PSShape_Box:
            SetBoxStartPosDir(shape, rand, pos, dir);
            break;
        default:
            break;
    }
}

float4 LerpColorOverLifetime(float t, ColorOverLifetime col)
{
    if (t <= col.Times[1] && col.Times[1] >= EPSILON)
        return lerp(col.Colors[0], col.Colors[1], t / col.Times[1]);
    
    else if (t <= col.Times[2] && col.Times[2] >= EPSILON)
        return lerp(col.Colors[1], col.Colors[2], (t - col.Times[1]) / (col.Times[2] - col.Times[1]));
    
    else if (t <= col.Times[3] && col.Times[3] >= EPSILON)
        return lerp(col.Colors[2], col.Colors[3], (t - col.Times[2]) / (col.Times[3] - col.Times[2]));
    
    return lerp(col.Colors[3], float4(0.f, 0.f, 0.f, 1.f), (t - col.Times[3]) / (1.f - col.Times[3]));
}

StructuredBuffer<ParticleSystemInfo> gParticleSystem : register(t0, space0);
RWStructuredBuffer<ParticleSharedInfo> gParticleShared : register(u0, space1);
RWStructuredBuffer<ParticleInfo> gOutputParticles : register(u0, space2);
ConstantBuffer<ParticleSystemIndexInfo> gIndex : register(b0);

[numthreads(512, 1, 1)]
void CSParticle(int3 threadID : SV_DispatchThreadID)
{
    ParticleSystemInfo ps = gParticleSystem[gIndex.Index];
    
    // 랜덤 시드 설정
    NumberGenerator rand;
    rand.SetSeed((threadID.x + 1) * (gIndex.Index + 1));
    
    // 최대 파티클을 벗어난 경우 리턴
    if (threadID.x >= ps.MaxParticles)
        return;

    // 파티클 공유 변수
    gParticleShared[gIndex.Index].AddCount = ps.AddCount;
    GroupMemoryBarrierWithGroupSync();
    
    // 파티클이 삭제된 경우에만 생성
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
		
        // 파티클 초기화
        if (gOutputParticles[threadID.x].Alive == 1)
        {
            // Start 초기화
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
            gOutputParticles[threadID.x].StartColor = SetStartColor(ps.StartColor, ps.TotalTime, rand.GetCurrentFloat());
            SetStartPosDir(ps.Shape, rand, gOutputParticles[threadID.x].LocalPos, gOutputParticles[threadID.x].WorldDir);
            
            if (ps.RotationOverLifetime.ValueOption == PSValue_Constant)
                gOutputParticles[threadID.x].AngularVelocity = ps.RotationOverLifetime.FirstAngularVelocity;
            else if (ps.RotationOverLifetime.ValueOption == PSValue_RandomBetweenTwoConstants)
                gOutputParticles[threadID.x].AngularVelocity = rand.GetRandomFloat(ps.RotationOverLifetime.FirstAngularVelocity, ps.RotationOverLifetime.SecondAngularVelocity);

            // Final 초기화
            gOutputParticles[threadID.x].FinalSize = gOutputParticles[threadID.x].StartSize;
            gOutputParticles[threadID.x].FinalColor = gOutputParticles[threadID.x].StartColor;
            gOutputParticles[threadID.x].FinalRotation = gOutputParticles[threadID.x].StartRotation;
        }
    }
    if (gOutputParticles[threadID.x].Alive == 1)
    {
        // 파티클 현재 시간 계산
        gOutputParticles[threadID.x].CurTime += ps.DeltaTime;
        
        // 파티클 현재 시간이 생명보다 크면 삭제
        if (gOutputParticles[threadID.x].LifeTime < gOutputParticles[threadID.x].CurTime)
        {
            gOutputParticles[threadID.x].Alive = 0;
            return;
        }
        
        // 파티클 현재 시간을 0~1로 정규화
        float lifeRatio = gOutputParticles[threadID.x].CurTime / gOutputParticles[threadID.x].LifeTime;

        float3 velocityOverLifetime = float3(0.f, 0.f, 0.f);
        if (ps.VelocityOverLifetime.IsOn == true)
        {
            velocityOverLifetime = ps.VelocityOverLifetime.Linear * ps.VelocityOverLifetime.SpeedModifier * lifeRatio * ps.DeltaTime;
        }
        
        if (ps.ColorOverLifetime.IsOn == true)
        {
            gOutputParticles[threadID.x].FinalColor = gOutputParticles[threadID.x].StartColor * LerpColorOverLifetime(lifeRatio, ps.ColorOverLifetime);
        }
        
        if (ps.SizeOverLifetime == true)
        {
            gOutputParticles[threadID.x].FinalSize = gOutputParticles[threadID.x].StartSize * lifeRatio;
        }
        
        if (ps.RotationOverLifetime.IsOn == true)
        {
            gOutputParticles[threadID.x].FinalRotation.y += gOutputParticles[threadID.x].AngularVelocity * ps.DeltaTime;
        }
        
        // 속도와 중력 계산
        float speed = gOutputParticles[threadID.x].StartSpeed * ps.DeltaTime;
        float3 gravity = float3(0.f, -9.81f * ps.GravityModifier, 0.f) * pow(lifeRatio, 2) * ps.SimulationSpeed * ps.DeltaTime;
        
        // 속도와 중력을 파티클 로컬 포지션에 적용
        gOutputParticles[threadID.x].LocalPos += normalize(gOutputParticles[threadID.x].WorldDir) * speed + velocityOverLifetime + gravity;
        
        // 이전 프레임의 위치 저장
        float3 prevPos = gOutputParticles[threadID.x].WorldPos;
        
        // SimulationSpace가 Local인 경우 프레임마다 물고 있는 오브젝트 위치 적용, World인 경우 생성시에만 적용
        if (ps.SimulationSpace == gkSimulationSpace_Local)
            gOutputParticles[threadID.x].WorldPos = gOutputParticles[threadID.x].LocalPos + ps.WorldPos;
        else if (ps.SimulationSpace == gkSimulationSpace_World)
            gOutputParticles[threadID.x].WorldPos = gOutputParticles[threadID.x].LocalPos + gOutputParticles[threadID.x].StartPos;

        // 현재 프레임과 이전 프레임에 대한 이동 방향 저장
        gOutputParticles[threadID.x].MoveDir = normalize(gOutputParticles[threadID.x].WorldPos - prevPos);
    }
	
}