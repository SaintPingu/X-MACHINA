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
    
    // 알파 값이 충분히 작다면 그림자 맵에 쓰지 않는다.
    clip(diffuseAlbedo.a - 0.1f);
    
    // 디졸브 값의 알파 값이 충분히 작다면 그림자 맵에 쓰지 않는다.
    clip(Dissolve((float3)0, gTextureMaps[gPassCB.LiveObjectDissolveIndex].Sample(gsamAnisotropicWrap, pin.UV).x, gObjectCB.DeathElapsed).a - 0.1f);
}