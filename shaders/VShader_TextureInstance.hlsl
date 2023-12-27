#include "VSResource.hlsl"


struct VS_TEX_INSTANCING_INPUT
{
    float3 position : POSITION;
    float2 uv : UV;
};
struct VS_TEX_INSTANCING_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITIONW;
    float2 uv : UV;
};

VS_TEX_INSTANCING_OUTPUT VSTextureInstancing(VS_TEX_INSTANCING_INPUT input, uint nInstanceID : SV_InstanceID)
{
    VS_TEX_INSTANCING_OUTPUT output;
    output.positionW = mul(float4(input.position, 1.0f), colorInstanceBuffer[nInstanceID].m_mtxGameObject).xyz;
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    return (output);
}
