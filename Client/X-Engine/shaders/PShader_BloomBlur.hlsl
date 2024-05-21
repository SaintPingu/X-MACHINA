#include "Common.hlsl"

static const int gBlurRadius = 5;

struct VSOutput_BloomBlur {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSBloomBlur(VSOutput_BloomBlur pin) : SV_TARGET
{
    float blurWeights[12] =
    {
        0.0561f, 0.1353f, 0.278f, 0.4868f,
        0.7261f, 0.9231f, 0.9231f, 0.7261f,
        0.4868f, 0.278f, 0.1353f, 0.0561f
    };
    
    float2 texOffset;
    if (gBloomCB.IsVertBlur)
    {
        texOffset = float2(0.f, gBloomCB.InvRenderTargetSize.y);

    }
    else
    {
        texOffset = float2(gBloomCB.InvRenderTargetSize.x, 0.f);
    }
    
    // 필터 핵 중앙의 값은 항상 총합에 기여한다.
    float4 color = blurWeights[gBlurRadius] * gTextureMaps[gBloomCB.PrevTargetIndex].SampleLevel(gsamPointClamp, pin.UV, 0.f);
    float totalWeight = blurWeights[gBlurRadius];

    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        // 중앙의 값은 이미 합산하였다.
        if (i == 0)
            continue;
        
        // 주변 픽셀의 UV 값을 구한다.
        float2 tex = pin.UV + i * texOffset;
        
        float weight = blurWeights[i + gBlurRadius];
        
        // 이웃 픽셀을 추가하여 흐린다.
        color += weight * gTextureMaps[gBloomCB.PrevTargetIndex].SampleLevel(gsamPointClamp, tex, 0.f);
        totalWeight += weight;
    }
    
    // 계산에서 제외된 표본이 있을 수 있기 때문에 실제 적용된 가중치 합으로 나눈다.
    return color / totalWeight;
}