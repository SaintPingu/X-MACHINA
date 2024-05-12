#include "Common.hlsl"

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

float4 PSMinimapAbility(VSOutput_Tex pin) : SV_TARGET0
{
    float4 ui = gTextureMaps[gObjectCB.MatIndex].Sample(gsamAnisotropicWrap, pin.UV);
    ui.a = 0.8f;
    
    float2 playerPos = gAbilityCB.MinimapData.PlayerPos;
    float2 minimapSize = gAbilityCB.MinimapData.MinimapSize;
    float2 minimapWorldSize = gAbilityCB.MinimapData.MinimapWorldSize;
    float2 minimapStartPos = gAbilityCB.MinimapData.MinimapStartPos;
    
    float2 playerUV = float2((playerPos.x - minimapStartPos.x) / minimapWorldSize.x, (playerPos.y - minimapStartPos.y) / minimapWorldSize.y);
    playerUV.y = 1 - playerUV.y;
    float2 v = pin.UV - playerUV;
    v.x *= minimapSize.x / minimapSize.y;
    
    if (length(v) < 0.01f)
    {
        return float4(0, 1, 0, 1);
    }
    
    return ui;
}
