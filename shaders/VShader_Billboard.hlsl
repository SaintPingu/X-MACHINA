#include "VSResource.hlsl"

struct VS_STANDARD_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VS_BILLBOARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITIONW;
    float2 uv : UV;
};



VS_BILLBOARD_OUTPUT VSBillboard(VS_STANDARD_INPUT input)
{
    VS_BILLBOARD_OUTPUT output;
    
    output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    
    return (output);
}

VS_BILLBOARD_OUTPUT VSSprite(VS_STANDARD_INPUT input)
{
    VS_BILLBOARD_OUTPUT output;

    output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = mul(float3(input.uv, 1.0f), (float3x3) (gmtxSprite)).xy;

    return (output);
}