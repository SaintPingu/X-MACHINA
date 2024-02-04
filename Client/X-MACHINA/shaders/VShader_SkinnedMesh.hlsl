#include "VSResource.hlsl"

struct VSInput_Standard {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : UV;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
    int4 Indices : BONEINDEX;
    float4 Weights : BONEWEIGHT;
};

struct VSOutput_Standard {
    float4 Position : SV_POSITION;
    float3 PositionW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV : UV;
    bool IsTexture : ISTEXTURE;
};

VSOutput_Standard VS_SkinnedMesh(VSInput_Standard input)
{
    VSOutput_Standard output;
    
    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.Weights[i] * mul(gpmtxBoneOffsets[input.Indices[i]], gpmtxBoneTransforms[input.Indices[i]]);
    }

    output.PositionW = (float3) mul(float4(input.Position, 1.f), mtxVertexToBoneWorld);
    output.Position = mul(mul(float4(output.PositionW, 1.f), gMtxView), gMtxProj);
    output.NormalW = mul(input.Normal, (float3x3) mtxVertexToBoneWorld);
    output.TangentW = (float3) mul(float4(input.Tangent, 1.f), mtxVertexToBoneWorld);
    output.BiTangentW = (float3) mul(float4(input.BiTangent, 1.f), mtxVertexToBoneWorld);
    output.UV = input.UV;
    
    if (gTextureMask > 0)
    {
        output.IsTexture = true;
    }
    else
    {
        output.IsTexture = false;
    }

    return output;
}