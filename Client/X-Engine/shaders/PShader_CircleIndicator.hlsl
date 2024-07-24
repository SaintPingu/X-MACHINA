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
    
    // �߰��Ǵ� ����
    float4 additiveColor    = float4(2.f, 0.5f, 0.2f, 0.f);
    float4 diffuseMapSample = (float4)0;
    
    // �ؽ�ó �߽����κ��� �Ÿ� ���
    const float dis = distance(pin.UV, float2(0.5f, 0.5f));
    const float lifeRatio = gAbilityCB.AccTime / gAbilityCB.ActiveTime;

    // ���� �ֱ�� �ȼ� �߽ɰ��� �Ÿ� ���̸� �̿��Ͽ� �ٱ����� ���������� �ִϸ��̼�
    if (dis < lifeRatio)
    {
        if (diffuseMapIndex != -1)
        {
            diffuseMapSample = GammaDecoding(gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.UV));
        }
    }
    
    // ���������� �ִϸ��̼ǿ� �ش�Ǵ� �ؽ�ó�� �ƴ� �⺻ ���� ����
    const float adjValue = 0.038f; // �ؽ�ó�� ��Ȯ�� 0.5 �Ÿ��� �ƴϱ� ������ ������ ���
    if (dis < (0.5f - adjValue))
    {
        // 0.5 ������ �Ÿ����� �ּ��� ���İ� ����
        additiveColor.a = 0.2f * (dis / 0.5f);
    }
    
    // �ؽ�ó �߽ɰ� ���� ���� �ֱ��� �� �̿��Ͽ� �� �ֺ��� ���ϰ�, �ּ��� ���ϰ� ���İ� ����
    float disRatio = dis / lifeRatio;
    diffuseMapSample.a *= pow(disRatio, 2.f);
    
    // �� ����� ���� ���� ����, ���� ������ �ð��� ������ ����
    return (diffuseMapSample + additiveColor * (1 - lifeRatio));
}
