#include "Common.hlsl"

cbuffer BlurInfo : register(b0)
{
	float gTotalTime;
    int gFilterOption;
};

Texture2D gInput				  : register(t0);
Texture2D gLUTTexture0		      : register(t1);
Texture2D gLUTTexture1		      : register(t2);
RWTexture2D<float4> gOutput		  : register(u0);

[numthreads(256, 1, 1)]
void LUTCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    float4 color = gInput[float2(dispatchThreadID.x, dispatchThreadID.y)];
 
    // 톤매핑
    if (gFilterOption & Filter_Tone)
    {
        float a = 2.51f;
        float b = 0.03f;
        float c = 2.43f;
        float d = 0.59f;
        float e = 0.14f;
        color = GammaEncoding(saturate((color * (a * color + b)) / (color * (c * color + d) + e)));
    }
    
    if (gFilterOption & Filter_LUT)
    {
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
        
        // 태양의 위치와 방향에 따라 lerp에 값을 넣어줘야 한다.
        float3 leftColor = lerp(leftColor0, leftColor1, cos(gTotalTime) * 0.5f + 0.5f);
        float3 rightColor = lerp(rightColor0, rightColor1, cos(gTotalTime) * 0.5f + 0.5f);
        
        color = float4(lerp(leftColor, rightColor, frac(color.rgb * 15.f)), color.a);
    }
    
    gOutput[dispatchThreadID.xy] = color;
}
