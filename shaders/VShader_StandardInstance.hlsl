#include "VSResource.hlsl"

struct VS_STANDARD_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : UV;
    bool isTexture : ISTEXTURE;
};




VS_STANDARD_OUTPUT VS_StandardInstance(VS_STANDARD_INPUT input, uint nInstanceID : SV_InstanceID)
{
    VS_STANDARD_OUTPUT output;

    matrix mtxWorld = instanceBuffer[nInstanceID].m_mtxGameObject;
    output.positionW = mul(mul(float4(input.position, 1.0f), gmtxWorld), mtxWorld).xyz;
    output.normalW = mul(input.normal, (float3x3) mtxWorld);
    output.tangentW = (float3) mul(float4(input.tangent, 1.0f), mtxWorld);
    output.bitangentW = (float3) mul(float4(input.bitangent, 1.0f), mtxWorld);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;
    
    if (texturesMask > 0)
    {
        output.isTexture = true;
    }
    else
    {
        output.isTexture = false;
    }

    return (output);
}