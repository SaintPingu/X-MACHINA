#include "Common.hlsl"

struct VSInput_Standard {
    float3 Position : POSITION;
    float2 UV : UV;
    int4 Indices : BONEINDEX;
    float4 Weights : BONEWEIGHT;
};

struct VSOutput_Standard
{
    float4 PosH : SV_POSITION;
    float2 UV : UV;
};

VSOutput_Standard VSShadowSkinnedMesh(VSInput_Standard input)
{
    VSOutput_Standard output;
    
    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.Weights[i] * gSkinMeshCB.BoneTransforms[input.Indices[i]];
    }

    float4 posW =  mul(float4(input.Position, 1.f), mtxVertexToBoneWorld);
    output.PosH = mul(mul(posW, gPassCB.MtxView), gPassCB.MtxProj);
    output.UV = input.UV;

    return output;
}