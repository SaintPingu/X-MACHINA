#include "VSResource.hlsl"

struct VS_INSTANCING_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};
struct VS_INSTANCING_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITIONW;
    float4 color : COLOR;
};


VS_INSTANCING_OUTPUT VSInstancing(VS_INSTANCING_INPUT input, uint nInstanceID : SV_InstanceID)
{
    VS_INSTANCING_OUTPUT output;
    output.positionW = mul(float4(input.position, 1.0f), colorInstanceBuffer[nInstanceID].m_mtxGameObject).xyz;
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.color = colorInstanceBuffer[nInstanceID].color;
    return (output);
}
