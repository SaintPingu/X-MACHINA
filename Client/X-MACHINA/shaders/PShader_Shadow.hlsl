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
	
	// Dynamically look up the texture in the array.
	diffuseAlbedo *= gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    
    clip(diffuseAlbedo.a - 0.1f);
}