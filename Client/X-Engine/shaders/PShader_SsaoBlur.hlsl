#include "Common.hlsl"

static const int gBlurRadius = 5;

struct VSOutput_SSAO {
    float4 PosH : SV_POSITION;
    float3 PosV : POSITION;
    float2 UV   : UV;
};

float NdcDepthToViewDepth(float zNdc)
{
    float viewZ = gPassCB.MtxProj[3][2] / (zNdc - gPassCB.MtxProj[2][2]);
    return viewZ;
}

float4 PSSsaoBlur(VSOutput_SSAO pin) : SV_TARGET
{
    float blurWeights[12] =
    {
        gSsaoCB.BlurWeights[0].x, gSsaoCB.BlurWeights[0].y, gSsaoCB.BlurWeights[0].z, gSsaoCB.BlurWeights[0].w,
        gSsaoCB.BlurWeights[1].x, gSsaoCB.BlurWeights[1].y, gSsaoCB.BlurWeights[1].z, gSsaoCB.BlurWeights[1].w,
        gSsaoCB.BlurWeights[2].x, gSsaoCB.BlurWeights[2].y, gSsaoCB.BlurWeights[2].z, gSsaoCB.BlurWeights[2].w,
    };
    
    float2 texOffset;
    if (gSsaoBlurCB.IsHorzBlur)
    {
        texOffset = float2(gSsaoCB.InvRenderTargetSize.x, 0.f);
    }
    else
    {
        texOffset = float2(0.f, gSsaoCB.InvRenderTargetSize.y);
    }
    
    // 필터 핵 중앙의 값은 항상 총합에 기여한다.
    float4 color = blurWeights[gBlurRadius] * gTextureMaps[gSsaoBlurCB.InputMapIndex].SampleLevel(gsamPointClamp, pin.UV, 0.f);
    float totalWeight = blurWeights[gBlurRadius];
    
    // 현재 픽셀(센터)의 노말과 깊이 값을 구한다.
    float3 centerNormal = gTextureMaps[gPassCB.RT1G_NormalIndex].SampleLevel(gsamPointClamp, pin.UV, 0.f).xyz;
    float centerDepth = NdcDepthToViewDepth(gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, pin.UV, 0.f).r);
    
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        // 중앙의 값은 이미 합산하였다.
        if (i == 0)
            continue;
        
        // 주변 픽셀의 UV 값을 구한다.
        float2 tex = pin.UV + i * texOffset;
        
        // 주변 픽셀의 노말 값과 깊이 값을 구한다.
        float3 neighborNormal = gTextureMaps[gPassCB.RT1G_NormalIndex].SampleLevel(gsamPointClamp, tex, 0.f).xyz;
        float neighborDepth = NdcDepthToViewDepth(gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, tex, 0.f).r);

        // 중앙의 값과 주변의 값의 노말, 깊이 차이가 크면 경계썬에 해당하기 때문에 흐리기에서 제외한다.
        if (dot(neighborNormal, centerNormal) >= 0.8f && abs(neighborDepth - centerDepth) <= 0.2f)
        {
            float weight = blurWeights[i + gBlurRadius];
            
            // 이웃 픽셀을 추가하여 흐린다.
            color += weight * gTextureMaps[gSsaoBlurCB.InputMapIndex].SampleLevel(gsamPointClamp, tex, 0.f);
            totalWeight += weight;
        }
    }
    
    // 계산에서 제외된 표본이 있을 수 있기 때문에 실제 적용된 가중치 합으로 나눈다.
    return color / totalWeight;
}