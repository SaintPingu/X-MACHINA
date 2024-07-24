#include "Light.hlsl"

struct VSOutput_Standard {
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float3 BiTangentW : BITANGENT;
    float2 UV         : UV;
};

float4 PSCircleIndicator(VSOutput_Standard pin) : SV_TARGET0
{
    // material info
    MaterialInfo matInfo  = gMaterialBuffer[gObjectCB.MatIndex];
    int diffuseMapIndex   = matInfo.DiffuseMap0Index;
    
    // 추가되는 색상
    float4 additiveColor    = float4(2.f, 0.5f, 0.2f, 0.f);
    float4 diffuseMapSample = (float4)0;
    
    // 텍스처 중심으로부터 거리 계산
    const float dis = distance(pin.UV, float2(0.5f, 0.5f));
    const float lifeRatio = gAbilityCB.AccTime / gAbilityCB.ActiveTime;

    // 생명 주기와 픽셀 중심과의 거리 차이를 이용하여 바깥으로 퍼져나가는 애니메이션
    if (dis < lifeRatio)
    {
        if (diffuseMapIndex != -1)
        {
            diffuseMapSample = GammaDecoding(gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
        }
    }
    
    // 퍼져나가는 애니메이션에 해당되는 텍스처가 아닌 기본 장판 색상
    const float adjValue = 0.038f; // 텍스처가 정확히 0.5 거리가 아니기 때문에 조정값 사용
    if (dis < (0.5f - adjValue))
    {
        // 0.5 떨어진 거리에서 멀수록 알파값 감소
        additiveColor.a = 0.2f * (dis / 0.5f);
    }
    
    // 텍스처 중심과 현재 생명 주기의 비를 이용하여 띠 주변은 진하게, 멀수록 연하게 알파값 조정
    float disRatio = dis / lifeRatio;
    diffuseMapSample.a *= pow(disRatio, 2.f);
    
    // 띠 색상과 장판 색상 조합, 장판 색상은 시간이 갈수록 감소
    return (diffuseMapSample + additiveColor * (1 - lifeRatio));
}
