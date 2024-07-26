#include "Common.hlsl"

struct VSInput_Standard {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : UV;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
    int4 Indices : BONEINDEX;
    float4 Weights : BONEWEIGHT;
};

struct VSOutput_Standard
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV : UV;
};

VSOutput_Standard VS_SkinnedMesh(VSInput_Standard input)
{
    VSOutput_Standard output;
    
    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    
    if (gObjectCB.IsSkinMesh)
    {
        for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
        {
            mtxVertexToBoneWorld += input.Weights[i] * gSkinMeshCB.BoneTransforms[input.Indices[i]];
        }
    }
    else
    {
        mtxVertexToBoneWorld = gObjectCB.MtxWorld;
    }

    output.PosW = (float3) mul(float4(input.Position, 1.f), mtxVertexToBoneWorld);
    output.PosH = mul(mul(float4(output.PosW, 1.f), gPassCB.MtxView), gPassCB.MtxProj);
    output.NormalW = mul(input.Normal, (float3x3) mtxVertexToBoneWorld);
    output.TangentW = (float3) mul(float4(input.Tangent, 1.f), mtxVertexToBoneWorld);
    output.BiTangentW = (float3) mul(float4(input.BiTangent, 1.f), mtxVertexToBoneWorld);
    output.UV = input.UV;

    return output;
}