#include "Common.hlsl"

#define EPSILON 0.0001

#define gkSimulationSpace_Local 0
#define gkSimulationSpace_World 1

#define PSVal_Constant                       0
#define PSVal_Curve                          1
#define PSVal_RandomBetweenTwoConstants      2
#define PSVal_RandomBetweenTwoCurve          3

#define PSShape_None        0
#define PSShape_Sphere      1
#define PSShape_HemiSphere  2
#define PSShape_Cone        3
#define PSShape_Box         4
#define PSShape_Line        5

#define CruveCount 4

struct ParticleSystemIndexInfo
{
	int Index;
};

struct ParticleSharedInfo
{
    int AddCount;
};

struct PSFloat
{
	uint    Option;
	uint    IsOn;
    float   Param1;
    float   Param2;
    float   Vals[CruveCount];
	float   ValKeys[CruveCount];
};

struct PSVec4
{
	uint    Option;
	uint    IsOn;
    float   Param1;
    float   Param2;
    float4  Vals[CruveCount];
	float   ValKeys[CruveCount];
};

struct PSColor
{
    uint    Option;
	uint    IsOn;
    float   Param1;
    float   Param2;
    float4  Vals[CruveCount];
	float   ValKeys[CruveCount];
	float   Alphas[CruveCount];
	float   AlphaKeys[CruveCount];
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

struct ParticleSystemInfo
{
    float3          WorldPos;
    int	            TextureIndex;
	float4          Color;
    
	int		        AddCount;
	int		        MaxParticles;
	float	        DeltaTime;
	float	        TotalTime;
    
	float2	        StartLifeTime;
	float2	        StartSpeed;
	float4	        StartSize3D;
	float4	        StartRotation3D;
	float2	        StartSize;
	float2	        StartRotation;
    PSColor         StartColor;
    
	float	        GravityModifier;
    uint            SimulationSpace;
    float           SimulationSpeed;
    float           Duration;
    
	PSVec4	        VelocityOverLifetime;
    PSColor         ColorOverLifetime;
    PSFloat         SizeOverLifetime;
    PSFloat         RotationOverLifetime;
    PSShape         Shape;
};

float4 SetStartColor(PSColor color, float t, float maxT, float r)
{
    float4 result = float4(1.f, 1.f, 1.f, 1.f);
    switch (color.Option)
    {
        case PSVal_Constant:
            result = color.Vals[0];
            break;
        case PSVal_Curve:
            result = lerp(color.Vals[0], color.Vals[1], frac(t / maxT));
            break;
        case PSVal_RandomBetweenTwoConstants:
            result = lerp(color.Vals[0], color.Vals[1], r);
            break;
        case PSVal_RandomBetweenTwoCurve:
            float4 firstColor = lerp(color.Vals[0], color.Vals[2], frac(t / 2));
            float4 secondColor = lerp(color.Vals[1], color.Vals[3], frac(t / 2));
            result = lerp(firstColor, secondColor, r);
            break;
    }
    return result;
}

void SetSphereStartPosDir(PSShape shape, inout NumberGenerator rand, inout float3 pos, inout float3 dir)
{
    // 파티클 초기 위치 설정
    float sphereCenter          = 0.f;
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
    float sphereCenter = 0.f;
    float sphereRadius = shape.Radius;
    float sphereRadiusThickness = shape.RadiusThickness;
    
    float theta = 2.f * 3.141592 * rand.GetCurrentFloat();
    float phi = acos(2.f * rand.GetCurrentFloat() - 1.f);
    float sinPhi = sin(phi);
    
    float r = rand.GetRandomFloat(sphereRadius * (1 - sphereRadiusThickness), sphereRadius);
    
    float x = r * sinPhi * cos(theta);
    float y = 0.f;
    float z = r * sinPhi * sin(theta);
    
    pos = sphereCenter + float3(x, y, z);
    
    dir = normalize(pos - sphereCenter);
}

void SetBoxStartPosDir(PSShape shape, inout NumberGenerator rand, inout float3 pos, inout float3 dir)
{
    
}

void SetLineStartPosDir(float3 startPos, float3 endPos, inout NumberGenerator rand, inout float3 pos, inout float3 dir)
{
    float3 vec = endPos - startPos;
    dir = normalize(vec);
    
    float t = rand.GetRandomFloat(-1.f, 0.f);

    pos = (vec * t);
    pos.xyz += rand.GetRandomFloat3(-0.1f, 0.1f);

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

float4 EvaluateCurveFloat4(float t, float4 vs[CruveCount], float ks[CruveCount])
{
    float4 val = float4(1, 1, 1, 1);
    
    for (int i = 0; i < CruveCount - 1; i++) {
        if (t < ks[i+1]) {
            float nT = (t - ks[i]) / (ks[i+1] - ks[i]);
            val = lerp(vs[i], vs[i+1], nT);
            break;
        }
    }
    
    return val;
}

float EvaluateCurveFloat1(float t, float vs[CruveCount], float ks[CruveCount])
{
    float val = 1.f;
    
    for (int i = 0; i < CruveCount - 1; i++) {
        if (t < ks[i+1]) {
            float nT = (t - ks[i]) / (ks[i+1] - ks[i]);
            val = lerp(vs[i], vs[i+1], nT);
            break;
        }
    }
    
    return val;
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
            gOutputParticles[threadID.x].StartColor = SetStartColor(ps.StartColor, ps.TotalTime, ps.Duration, rand.GetCurrentFloat());
            if (ps.Shape.ShapeType == PSShape_Line)
            {
                SetLineStartPosDir(ps.StartRotation3D.xyz, ps.StartSize3D.xyz, rand, gOutputParticles[threadID.x].LocalPos, gOutputParticles[threadID.x].WorldDir);
            }
            else
            {
                SetStartPosDir(ps.Shape, rand, gOutputParticles[threadID.x].LocalPos, gOutputParticles[threadID.x].WorldDir);
            }
            
            if (ps.VelocityOverLifetime.Option == PSVal_Constant)
                gOutputParticles[threadID.x].VelocityOverLifetime.xyz = ps.VelocityOverLifetime.Vals[0].xyz;
            else if (ps.VelocityOverLifetime.Option == PSVal_RandomBetweenTwoConstants)
            {
                gOutputParticles[threadID.x].VelocityOverLifetime.x = rand.GetRandomFloat(ps.VelocityOverLifetime.Vals[0].x, ps.VelocityOverLifetime.Vals[1].x);
                gOutputParticles[threadID.x].VelocityOverLifetime.y = rand.GetRandomFloat(ps.VelocityOverLifetime.Vals[0].y, ps.VelocityOverLifetime.Vals[1].y);
                gOutputParticles[threadID.x].VelocityOverLifetime.z = rand.GetRandomFloat(ps.VelocityOverLifetime.Vals[0].z, ps.VelocityOverLifetime.Vals[1].z);
            }

            if (ps.RotationOverLifetime.Option == PSVal_Constant)
                gOutputParticles[threadID.x].AngularVelocity = ps.RotationOverLifetime.Vals[0];
            else if (ps.RotationOverLifetime.Option == PSVal_RandomBetweenTwoConstants)
                gOutputParticles[threadID.x].AngularVelocity = rand.GetRandomFloat(ps.RotationOverLifetime.Vals[0], ps.RotationOverLifetime.Vals[1]);

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
        if (ps.VelocityOverLifetime.IsOn == TRUE)
        {
            velocityOverLifetime = gOutputParticles[threadID.x].VelocityOverLifetime.xyz * ps.VelocityOverLifetime.Param1 * lifeRatio * ps.DeltaTime;
        }
        
        if (ps.ColorOverLifetime.IsOn == TRUE)
        {
            gOutputParticles[threadID.x].FinalColor.xyz = gOutputParticles[threadID.x].StartColor.xyz * EvaluateCurveFloat4(lifeRatio, ps.ColorOverLifetime.Vals, ps.ColorOverLifetime.ValKeys).xyz;
            gOutputParticles[threadID.x].FinalColor.a = gOutputParticles[threadID.x].StartColor.a * EvaluateCurveFloat1(lifeRatio, ps.ColorOverLifetime.Alphas, ps.ColorOverLifetime.AlphaKeys);
        }
        
        if (ps.SizeOverLifetime.IsOn == TRUE)
        {
            gOutputParticles[threadID.x].FinalSize = gOutputParticles[threadID.x].StartSize * EvaluateCurveFloat1(lifeRatio, ps.SizeOverLifetime.Vals, ps.SizeOverLifetime.ValKeys);
        }
        
        if (ps.RotationOverLifetime.IsOn == TRUE)
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