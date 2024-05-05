#include "Common.hlsl"

struct VSOutput_Shadow {
    float4 PosH : SV_POSITION;
    float2 UV : UV;
};

void PSShadow(VSOutput_Shadow pin)
{
    MaterialInfo matInfo = gMaterialBuffer[gObjectCB.MatIndex];
	float4 diffuseAlbedo = matInfo.Diffuse;
    int diffuseMapIndex = matInfo.DiffuseMap0Index;
	
    // ������ ���� ���� ���� ����� �۴ٸ� �׸��� �ʿ� ���� �ʴ´�.
	diffuseAlbedo *= gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    diffuseAlbedo.a -= (1 - Dissolve((float3)0, gTextureMaps[gPassCB.LiveObjectDissolveIndex].Sample(gsamAnisotropicWrap, pin.UV * 2.f).x, gObjectCB.DeathElapsed).a);
    
    // ���� ���� ����� �۴ٸ� �׸��� �ʿ� ���� �ʴ´�.
    clip(diffuseAlbedo.a - 0.1f);
}