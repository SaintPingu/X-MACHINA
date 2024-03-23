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
    
    // ���� �� �߾��� ���� �׻� ���տ� �⿩�Ѵ�.
    float4 color = blurWeights[gBlurRadius] * gTextureMaps[gSsaoBlurCB.InputMapIndex].SampleLevel(gsamPointClamp, pin.UV, 0.f);
    float totalWeight = blurWeights[gBlurRadius];
    
    // ���� �ȼ�(����)�� �븻�� ���� ���� ���Ѵ�.
    float3 centerNormal = gTextureMaps[gPassCB.RT1G_NormalIndex].SampleLevel(gsamPointClamp, pin.UV, 0.f).xyz;
    float centerDepth = NdcDepthToViewDepth(gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, pin.UV, 0.f).r);
    
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        // �߾��� ���� �̹� �ջ��Ͽ���.
        if (i == 0)
            continue;
        
        // �ֺ� �ȼ��� UV ���� ���Ѵ�.
        float2 tex = pin.UV + i * texOffset;
        
        // �ֺ� �ȼ��� �븻 ���� ���� ���� ���Ѵ�.
        float3 neighborNormal = gTextureMaps[gPassCB.RT1G_NormalIndex].SampleLevel(gsamPointClamp, tex, 0.f).xyz;
        float neighborDepth = NdcDepthToViewDepth(gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, tex, 0.f).r);

        // �߾��� ���� �ֺ��� ���� �븻, ���� ���̰� ũ�� ���㿡 �ش��ϱ� ������ �帮�⿡�� �����Ѵ�.
        if (dot(neighborNormal, centerNormal) >= 0.8f && abs(neighborDepth - centerDepth) <= 0.2f)
        {
            float weight = blurWeights[i + gBlurRadius];
            
            // �̿� �ȼ��� �߰��Ͽ� �帰��.
            color += weight * gTextureMaps[gSsaoBlurCB.InputMapIndex].SampleLevel(gsamPointClamp, tex, 0.f);
            totalWeight += weight;
        }
    }
    
    // ��꿡�� ���ܵ� ǥ���� ���� �� �ֱ� ������ ���� ����� ����ġ ������ ������.
    return color / totalWeight;
}