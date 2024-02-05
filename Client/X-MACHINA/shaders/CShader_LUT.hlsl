#include "Common.hlsl"

cbuffer BlurInfo : register(b0)
{
	float gTotalTime;
};

Texture2D gInput				  : register(t0);
Texture2D gLUTTexture0		      : register(t1);
Texture2D gLUTTexture1		      : register(t2);
RWTexture2D<float4> gOutput		  : register(u0);

[numthreads(256, 1, 1)]
void LUTCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    float4 color = gInput[float2(dispatchThreadID.x, dispatchThreadID.y)];
    
    float u = floor(color.b * 15.f) / 15.f * 240.f;
    u = (floor(color.r * 15.f) / 15.f * 15.f) + u;
    float v = floor(color.g * 15.f);
    float3 leftColor0 = gLUTTexture0[float2(u, v)].rgb;
    float3 leftColor1 = gLUTTexture1[float2(u, v)].rgb;
    
    u = ceil(color.b * 15.f) / 15.f * 240.f;
    u = (ceil(color.r * 15.f) / 15.f * 15.f) + u;
    v = ceil(color.g * 15.f);
    float3 rightColor0 = gLUTTexture0[float2(u, v)].rgb;
    float3 rightColor1 = gLUTTexture1[float2(u, v)].rgb;
    
    // �¾��� ��ġ�� ���⿡ ���� lerp�� ���� �־���� �Ѵ�.
    float3 leftColor = lerp(leftColor0, leftColor1, clamp(sin(gTotalTime), 0.f, 0.9f));
    float3 rightColor = lerp(rightColor0, rightColor1, clamp(sin(gTotalTime), 0.f, 0.9f));
    
    float3 result = lerp(leftColor, rightColor, frac(color.rgb * 15.f));
    
    //// �׽�Ʈ ACES_Filmic �����
    //// ����� R8G8B8A8 �ĸ� ���� �ؽ�ó�� �����ͼ� ����ϱ� ������ HDR�� ������ ���� ���� �����̴�.
    //// ���Ŀ� R16G16B16A16 ȭ�� �� �ؽ�ó�� �����ͼ� ����� �� HDR�� �����Ͽ� ������� ������ �� �ִ�.
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    result = saturate((result * (a * result + b)) / (result * (c * result + d) + e));
    
    gOutput[dispatchThreadID.xy] = float4(result, color.a);
}
