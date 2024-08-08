#include "Light.hlsl"

struct VSOutput_Standard {
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV         : UV;
};

float4 PSShieldAbility(VSOutput_Standard pin) : SV_TARGET0
{
    // material info
    MaterialInfo matInfo  = gMaterialBuffer[gObjectCB.MatIndex];
    float4 diffuse        = float4(0.4f, 0.4f, 0.4f, 0.8f);
    int diffuseMapIndex   = matInfo.DiffuseMap0Index;
    
    float rimWidth = 1.4f;
    float rim = 1.0f - max(0, abs(dot(pin.NormalW, normalize(gPassCB.CameraPos - pin.PosW))));
    rim = smoothstep(1.0f - rimWidth, 1.0f, rim);
    
    float4 opacitySample = (float4)0;
    if (diffuseMapIndex != -1)
    {
        float2 scaleUV = pin.UV * float2(5.f, 5.f);
        opacitySample = GammaDecoding(gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, scaleUV));
    }
    
    // depth fade
    float2 screenUV = float2(pin.PosH.x / gPassCB.FrameBufferWidth, pin.PosH.y / gPassCB.FrameBufferHeight);
    float depthViewZ = NdcDepthToViewDepth(gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, screenUV, 0.f).x);
    float pixelViewZ = NdcDepthToViewDepth(pin.PosH.z);
    float depthDiff = depthViewZ - pixelViewZ;
    float depthFade = saturate(pow(depthDiff, 4.f));
    
    float opacity = max(opacitySample, rim);
    float2 bandUV = pin.UV + float2(0.f, -gAbilityCB.AccTime * 0.5f);
    
    float4 litColor = GammaEncoding(lerp(diffuse, float4(1.f.xxx, 0.8f), rim));
    litColor.a = diffuse.a * max(opacitySample * GeneratedBand(bandUV, false, 0.25f, 0.25f, 0.8f), rim * 0.5f) * depthFade;
    
    // 시간에 따라 알파 값을 선형 감쇠
    litColor.a *= CalcAttenuation(gAbilityCB.AccTime, gAbilityCB.Duration, gAbilityCB.ActiveTime);
    
    return litColor;
}
