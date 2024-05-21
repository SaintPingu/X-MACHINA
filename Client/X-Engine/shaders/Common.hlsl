#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_

#define _WITH_LOCAL_VIEWER_HIGHLIGHT
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT
#define POST_PROCESSING

#define LightType_Spot           0
#define LightType_Directional    1
#define LightType_Point          2
#define gkMaxTextureCount        1024
#define gkMaxSkyBoxCount         16
#define gkMaxSceneLight          32
                                 
#define MAX_VERTEX_INFLUENCES	 4
#define SKINNED_ANIMATION_BONES  128
                                 
#define Filter_None              0x001
#define Filter_Blur              0x002
#define Filter_LUT               0x004
#define Filter_Tone              0x008
#define Filter_Ssao              0x010
#define Filter_Shadow            0x020
#define Filter_Custom            0x040

#define RANDOM_IA 16807
#define RANDOM_IM 2147483647
#define RANDOM_AM (1.0f/float(RANDOM_IM))
#define RANDOM_IQ 127773u
#define RANDOM_IR 2836
#define RANDOM_MASK 123459876

#define NONE    -1
#define FALSE   0
#define TRUE    1

struct LightInfo
{
    float3  Strength;
    float   FalloffStart;   // point/spot light only
    float3  Direction;      // directional/spot light only
    float   FalloffEnd;     // point/spot light only
    float3  Position;       // point light only
    float   SpotPower;      // spot light only
    int     LightType;
    int     IsEnable;
	int		ObjCBIndex;
    int     Padding;
};

struct LightColor
{
    float3  Diffuse;
    float3  Specular;
};

struct Material
{
    float3  DiffuseAlbedo;
    float3  SpecularAlbedo;
    float   Metallic;
    float   Roughness;
};

struct MaterialInfo {
    float4  Diffuse;
    float   Metallic;
    float   Roughness;
    uint    OcclusionMask;
    uint    Padding;
    
    int DiffuseMap0Index;
    int DiffuseMap1Index;
    int DiffuseMap2Index;
    int DiffuseMap3Index;
    
    int NormalMapIndex;
    int EmissiveMapIndex;
    int MetallicMapIndex;
    int OcclusionMapIndex;
};

struct ParticleInfo
{
	float3	StartPos;
	float	CurTime;
	float3	LocalPos;
	float	LifeTime;
	float3	WorldPos;
	int	    Alive;
	float3	WorldDir;
    int		TextureIndex;
	float3	MoveDir;
    float	StartSpeed;
	float3	StartRotation;
	float	Padding1;
    float3  FinalRotation;
    float   AngularVelocity;
	float2	StartSize;
	float2	FinalSize;
    float4  StartColor;
    float4  FinalColor;
    float4  VelocityOverLifetime;
};

struct ObjectInfo {
    matrix  MtxWorld;
    matrix  MtxSprite;
    int     MatIndex;
    int     LightIndex;
    float   DeathElapsed;
    float   RimFactor;
};

struct PassInfo {
    matrix      MtxView;
    matrix      MtxProj;
    matrix      MtxInvProj;
    matrix      MtxShadow;
    matrix      MtxNoLagView;
    float3      CameraPos;
    uint        LightCount;
    float3      CameraRight;
    int         FrameBufferWidth;
    float3      CameraUp;
    int         FrameBufferHeight;
    LightInfo   Lights[gkMaxSceneLight];
    
    float       DeltaTime;
    float       TotalTime;
    float       FogStart;
    float       FogRange;
    
    float4      GlobalAmbient;
    float4      FogColor;

    int         FilterOption;
    float       ShadowIntensity;
    
    int         SkyBoxIndex;
    int         DefaultDsIndex;
    int         ShadowDsIndex;
    int         CustomDsIndex;
    
    int         RT0G_PositionIndex;
    int         RT1G_NormalIndex;
    int         RT2G_DiffuseIndex;
    int         RT3G_EmissiveIndex;
    
    int         RT4G_MetallicSmoothnessIndex;
    int         RT5G_OcclusionIndex;
    int         RT0L_DiffuseIndex;
    int         RT1L_SpecularIndex;
    
    int         RT2L_AmbientIndex;
    int         RT0S_SsaoIndex;
    int         LiveObjectDissolveIndex;
    int         BuildingDissolveIndex;
    
    int         RT0O_OffScreenIndex;
    int         BloomIndex;
};

struct PostPassInfo {
    int         RT0_OffScreenIndex;
    float3      Padding;
};

struct ColliderInfo {
    matrix MtxView;
    float4 Color;
};

struct BoneTransformInfo {
    float4x4 BoneTransforms[SKINNED_ANIMATION_BONES];
};

struct SsaoInfo {
    matrix  MtxInvProj;
    matrix  MtxProjTex;
    float4  OffsetVectors[14];

    float4  BlurWeights[3];
    float2  InvRenderTargetSize;
    
    float   OcclusionRadius;
    float   OcclusionFadeStart;
    float   OcclusionFadeEnd;
    float   SurfaceEpsilon;
    
    int     AccessContrast;
    int     RandomVectorIndex;
};

struct SsaoBlurInfo {
    int     InputMapIndex;
    int     IsHorzBlur;
};

struct BloomInfo {
    int     PrevTargetIndex;
    int     BaseTargetIndex;
    float2  InvRenderTargetSize;
    int     IsVertBlur;
};

struct MinimapInfo {
    float2 PlayerPos;
    float2 MinimapStartPos;
	float2 MinimapSize;
	float2 MinimapWorldSize;
};

struct AbilityInfo {
	float   ActiveTime;
	float   AccTime;
    float   Duration;
    int     UIIndex;

    MinimapInfo MinimapData;
};

struct InstanceInfo {
    matrix  MtxObject;
};

ConstantBuffer<ObjectInfo> gObjectCB            : register(b0);
ConstantBuffer<PassInfo> gPassCB                : register(b1);
ConstantBuffer<PostPassInfo> gPostPassCB        : register(b2);
ConstantBuffer<ColliderInfo> gColliderCB        : register(b3);
ConstantBuffer<BoneTransformInfo> gSkinMeshCB   : register(b4);
ConstantBuffer<SsaoInfo> gSsaoCB                : register(b5);
ConstantBuffer<SsaoBlurInfo> gSsaoBlurCB        : register(b6);
ConstantBuffer<AbilityInfo> gAbilityCB          : register(b7);
ConstantBuffer<BloomInfo> gBloomCB              : register(b8);

StructuredBuffer<InstanceInfo> gInstBuffer   : register(t0);
StructuredBuffer<MaterialInfo> gMaterialBuffer  : register(t0, space1);
StructuredBuffer<ParticleInfo> gInputPraticles   : register(t0, space2);

TextureCube gSkyBoxMaps[gkMaxSkyBoxCount]       : register(t1, space1);
Texture2D gTextureMaps[gkMaxTextureCount]       : register(t1); // t1, t2, t3...

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);
SamplerState gsamDepthMap         : register(s7);

// 디스플레이 출력은 어두운 부분을 더 자세히 표현하기 위해서 이미지를 Decoding(어둡게)하여 출력한다.
// 이로 인해, 대부분의 텍스처는 전체적으로 어두운 부분을 해결하기 위해 Encoding(밝게)되어 저장된다.
// 따라서 빛, 조명과 관련된 연산은 비선형 공간이 아닌 선형 공간에서 처리해야 한다.

// 감마 보정을 적용하는 함수
float4 GammaEncoding(float4 color)
{
    return float4(pow(color.rgb, 1 / 2.2f), color.a);
}

float4 GammaEncodingAlpha(float4 color)
{
    return float4(pow(color, 1 / 2.2f));
}

// 감마 보정을 해제하는 함수
float4 GammaDecoding(float4 color)
{
    return float4(pow(color.rgb, 2.2f), color.a);
}

float4 GammaDecodingAlpha(float4 color)
{
    return float4(pow(color, 2.2f));
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
    // [0,1] 공간에서 [-1,1]로 매핑한다.
    float3 normalT = 2.0f * normalMapSample - 1.0;

    // orthonormal basis(TBN 행렬)를 생성한다.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

    // tangent 공간에서 world 공간으로 변환한다.
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

float Mix(float x, float y, float alpha)
{
    return x * (1.f - alpha) + y * x;
}

float4 Fog(float4 color, float3 position)
{
    float3 camPos = gPassCB.CameraPos;
    float3 posToCam = camPos - position;
    float distance = length(posToCam);
    
    float factor = min(saturate((distance - gPassCB.FogStart) / gPassCB.FogRange), 0.8f);
    return lerp(color, gPassCB.FogColor, factor);
}

float4 FogDistance(float4 color, float3 distance)
{
    float factor = (float) min(saturate((distance - gPassCB.FogStart) / gPassCB.FogRange), 0.8f);
    return lerp(color, gPassCB.FogColor, factor);
}

bool IsOutOfRange(float3 value, float lhs = 0.f, float rhs = 1.f)
{
    return (value.x < 0.f || value.x > 1.f ||
            value.y < 0.f || value.y > 1.f ||
            value.z < 0.f || value.z > 1.f);
}

float ComputeShadowFactor(float4 shadowPosH)
{
    // w로 나눠서 투영을 완료한다.
    shadowPosH.xyz /= shadowPosH.w;
    
    // 절두체 범위의 밖에 있다면 계산하지 않고 바로 리턴한다.
    if (IsOutOfRange(shadowPosH.xyz))
    {
        return 1.f;
    }
    else
    {
        // z값이 깊이 값이다.
        float depth = shadowPosH.z;
    
        uint width, height, numMips;
        gTextureMaps[gPassCB.ShadowDsIndex].GetDimensions(0, width, height, numMips);
    
        // 텍셀 사이즈
        float dx = 1.f / (float) width;
    
        // 비율 근접 필터링(PCF)
        float percentLit = 0.f;
        const float2 offsets[9] =
        {
            float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
        };
    
        [unroll]
        for (int i = 0; i < 9; ++i)
        {
            percentLit += gTextureMaps[gPassCB.ShadowDsIndex].SampleCmpLevelZero(gsamShadow, shadowPosH.xy + offsets[i], depth).r;
        }
    
        return percentLit / 9.f;
    }
}

// random generator
struct NumberGenerator
{
    int seed; // Used to generate values.

    // Generates the next number in the sequence.
    void Cycle()
    {
        seed ^= RANDOM_MASK;
        int k = seed / RANDOM_IQ;
        seed = RANDOM_IA * (seed - k * RANDOM_IQ) - RANDOM_IR * k;

        if (seed < 0) 
            seed += RANDOM_IM;

        seed ^= RANDOM_MASK;
    }
    
    // Returns the current random float.
    float GetCurrentFloat()
    {
        Cycle();
        return RANDOM_AM * seed;
    }

    // Returns the current random int.
    int GetCurrentInt()
    {
        Cycle();
        return seed;
    }

    // Cycles the generator based on the input count. Useful for generating a thread unique seed.
    // PERFORMANCE - O(N)
    void Cycle(const uint _count)
    {
        for (uint i = 0; i < _count; ++i)
            Cycle();
    }

    // Returns a random float within the input range.
    float GetRandomFloat(const float low, const float high)
    {
        if (low != high)
        {
            float v = GetCurrentFloat();
            return low * (1.0f - v) + high * v;
        }
        else
        {
            return low;
        }
    }
    
    // Returns a random float within the input range.
    float3 GetRandomFloat3(const float low, const float high)
    {
        if (low == high)
            return float3(low, low, low);
        
        float v = GetCurrentFloat();
        float x = low * (1.0f - v) + high * v;
        v = GetCurrentFloat();
        float y = low * (1.0f - v) + high * v;
        v = GetCurrentFloat();
        float z = low * (1.0f - v) + high * v;
        
        return float3(x, y, z);
    }

    // Sets the seed
    void SetSeed(const uint value)
    {
        seed = int(value);
        Cycle();
    }
};

void RotationVector(inout float3 right, inout float3 up, inout float3 look, float3 axis, float angle)
{
    if (angle == 0)
        return;
    
    // 축을 정규화
    axis = normalize(axis);

    // 회전 각도를 라디안으로 변환
    float cosTheta = cos(radians(angle));
    float sinTheta = sin(radians(angle));

    // 회전 행렬을 구성
    float3x3 rotationMatrix = float3x3(
        cosTheta + (1 - cosTheta) * axis.x * axis.x, 
        (1 - cosTheta) * axis.x * axis.y - sinTheta * axis.z, 
        (1 - cosTheta) * axis.x * axis.z + sinTheta * axis.y,

        (1 - cosTheta) * axis.y * axis.x + sinTheta * axis.z,
        cosTheta + (1 - cosTheta) * axis.y * axis.y, 
        (1 - cosTheta) * axis.y * axis.z - sinTheta * axis.x,

        (1 - cosTheta) * axis.z * axis.x - sinTheta * axis.y,
        (1 - cosTheta) * axis.z * axis.y + sinTheta * axis.x,
        cosTheta + (1 - cosTheta) * axis.z * axis.z
    );

    // 회전 행렬을 right, look, up 벡터에 적용
    right = mul(right, rotationMatrix);
    look = mul(look, rotationMatrix);
    up = mul(up, rotationMatrix);
}


void RotationAxis(inout float3 right, inout float3 up, inout float3 look, float3 angle)
{
    RotationVector(right, look, up, right, angle.x);
    RotationVector(right, look, up, look, angle.y);
    RotationVector(right, look, up, up, angle.z);
}


float4 Dissolve(float3 color, float dissolve, float t)
{
    float m1 = t;
    float m2 = 0.15f;
    float max = m1 * (m2 + 1);
    float min = max - m2;

    float smoothAlpha = smoothstep(min, max, dissolve);
    float3 smoothColor = (1 - smoothstep(min, max, dissolve)) * color;

    return float4(smoothColor, smoothAlpha);
}


float NdcDepthToViewDepth(float zNdc)
{
    float viewZ = gPassCB.MtxProj[3][2] / (zNdc - gPassCB.MtxProj[2][2]);
    return viewZ;
}


float4 ComputeRimLight(float4 rimLightColor, float rimWidth, float rimFactor, float3 posW, float3 normalW)
{
    float rim = 1.0f - max(0, dot(normalW, normalize(gPassCB.CameraPos - posW)));
    rim = smoothstep(1.0f - rimWidth, 1.0f, rim) * gObjectCB.RimFactor;
    
    return rim * rimLightColor;
}


float SphereMask(float4 coords, float4 center, float radius, float hardness = 1.f)
{
    return 1 - saturate((distance(coords, center) - radius) / (1 - hardness));
}


void ApplyOcculsionMaskByCamera(float3 posW, float2 uvW)
{
    float radius = 0.5f;
    float distance = 50.f;
    
    float yDist = abs(gPassCB.CameraPos.y - posW.y);
        
    float3 pinPosV = mul(float4(posW, 1.f), gPassCB.MtxNoLagView);
    float3 camPosV = mul(float4(gPassCB.CameraPos, 1.f), gPassCB.MtxNoLagView);
        
    float sphereMask = SphereMask(float4(pinPosV.xy, 0.f, 0.f), float4(camPosV.xy, 0.f, 0.f), radius, 0.1f);
    
    float3 distToCam = abs(pinPosV - camPosV);
    float lengthToCam = length(distToCam);
    
    float dissolve = gTextureMaps[gPassCB.BuildingDissolveIndex].Sample(gsamAnisotropicWrap, uvW + gPassCB.TotalTime * 0.03f).x;
    float sphereMask2 = sphereMask + dissolve;
    
    float mulMasks = sphereMask * sphereMask2;
    
    float lit = 0.f;
    if (distance >= lengthToCam)
    {
        lit = mulMasks;
    }
    else if (distance < lengthToCam)
    {
        lit = 1.f;
    }

    //if (yDist < 9.f)
        clip(0.1f - lit);
}

// 선형 감쇠 함수
float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

// 선형 강화 함수
float CalcAmplification(float d, float falloffStart, float falloffEnd)
{
    return saturate((d - falloffStart) / (falloffEnd - falloffStart));
}

// Direction == false : v(UV)
float GeneratedBand(float2 uv, bool DirectionSwitch, float4 center, float width, float hardness)
{
    float direction = DirectionSwitch ? frac(uv.x) : frac(uv.y);
    
    return SphereMask(direction, center, width, hardness);
}
#endif