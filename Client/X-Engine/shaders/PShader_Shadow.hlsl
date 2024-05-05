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
	
    // 디졸브 값의 알파 값이 충분히 작다면 그림자 맵에 쓰지 않는다.
	diffuseAlbedo *= gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV);
    diffuseAlbedo.a -= (1 - Dissolve((float3)0, gTextureMaps[gPassCB.LiveObjectDissolveIndex].Sample(gsamAnisotropicWrap, pin.UV * 2.f).x, gObjectCB.DeathElapsed).a);
    
    // 알파 값이 충분히 작다면 그림자 맵에 쓰지 않는다.
    clip(diffuseAlbedo.a - 0.1f);
}