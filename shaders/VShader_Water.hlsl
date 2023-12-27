#include "VSResource.hlsl"

struct VS_WATER_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VS_WATER_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float2 uv : UV;
};

cbuffer gameInfo : register(b3)
{
    float deltaTime : packoffset(c0);
};

VS_WATER_OUTPUT VSWater(VS_WATER_INPUT input)
{
    VS_WATER_OUTPUT output;
    input.position.y += sin(deltaTime * 1.35f + input.position.x * 1.35f) * 1.95f + cos(deltaTime * 1.30f + input.position.z * 1.35f) * 1.05f;

    output.positionW = (float3) mul(float4(input.position, 1.0f), gmtxWorld);
    output.normalW = mul(input.normal, (float3x3) gmtxWorld);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv * 5.0f;

    return (output);
}