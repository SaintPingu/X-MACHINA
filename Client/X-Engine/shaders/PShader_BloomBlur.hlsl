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
    
    // ���� �� �߾��� ���� �׻� ���տ� �⿩�Ѵ�.
    float4 color = blurWeights[gBlurRadius] * gTextureMaps[gBloomCB.PrevTargetIndex].SampleLevel(gsamPointClamp, pin.UV, 0.f);
    float totalWeight = blurWeights[gBlurRadius];

    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        // �߾��� ���� �̹� �ջ��Ͽ���.
        if (i == 0)
            continue;
        
        // �ֺ� �ȼ��� UV ���� ���Ѵ�.
        float2 tex = pin.UV + i * texOffset;
        
        float weight = blurWeights[i + gBlurRadius];
        
        // �̿� �ȼ��� �߰��Ͽ� �帰��.
        color += weight * gTextureMaps[gBloomCB.PrevTargetIndex].SampleLevel(gsamPointClamp, tex, 0.f);
        totalWeight += weight;
    }
    
    // ��꿡�� ���ܵ� ǥ���� ���� �� �ֱ� ������ ���� ����� ����ġ ������ ������.
    return color / totalWeight;
}