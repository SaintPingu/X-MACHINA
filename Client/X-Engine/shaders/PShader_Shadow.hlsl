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
	
	diffuseAlbedo *= gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    
    // ���� ���� ����� �۴ٸ� �׸��� �ʿ� ���� �ʴ´�.
    clip(diffuseAlbedo.a - 0.1f);
    
    // ������ ���� ���� ���� ����� �۴ٸ� �׸��� �ʿ� ���� �ʴ´�.
    clip(Dissolve((float3)0, gTextureMaps[gPassCB.LiveObjectDissolveIndex].Sample(gsamAnisotropicWrap, pin.UV).x, gObjectCB.DeathElapsed).a - 0.1f);
}