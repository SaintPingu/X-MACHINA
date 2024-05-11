#include "Common.hlsl"

static const int gSampleCount = 14;

struct VSOutput_SSAO {
    float4 PosH : SV_POSITION;
    float3 PosV : POSITION;
    float2 UV   : UV;
};

float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;
	if (distZ > gSsaoCB.SurfaceEpsilon)
    {
        float fadeLength = gSsaoCB.OcclusionFadeEnd - gSsaoCB.OcclusionFadeStart;
		
		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
        occlusion = saturate((gSsaoCB.OcclusionFadeEnd - distZ) / fadeLength);
    }
	
	return occlusion;	
}

float4 PSSsao(VSOutput_SSAO pin) : SV_TARGET0
{
    // �� ���������� �븻 ���� ���� ��
    float3 n = normalize(mul(gTextureMaps[gPassCB.RT1G_NormalIndex].SampleLevel(gsamPointClamp, pin.UV, 0.f), gPassCB.MtxView).xyz);
    float pz = gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, pin.UV, 0.f).r;
    pz = NdcDepthToViewDepth(pz);
    
    // ���� ó������ �ȼ��� �籸�� ����
    // p = t * v, pz = t * vz, t = pz / vz
    // p = (pz / vz) * v
    float3 p = (pz / pin.PosV.z) * pin.PosV;
    
    // ���� ���͸� ���ø��Ͽ� [0, 1]�� ���� [-1, 1]�� �����Ѵ�.
    // ���� ���� ���� ũ�Ⱑ �۱� ������ UV ���� 4�� ���Ͽ� ���ø��� ���� �ǵ��� �Ѵ�.
    float3 randVec = 2.f * gTextureMaps[gSsaoCB.RandomVectorIndex].SampleLevel(gsamLinearWrap, 4.f * pin.UV, 0.f).rgb - 1.f;
    
	float occlusionSum = 0.0f;
    
    for (int i = 0; i < gSampleCount; ++i)
    {
        // �̸� ������ ���� ������ ������ ���͸� �ϳ��� ������ ���� �������� �ݻ��Ų��.
        // �ƹ�ư ������ ���� ������ ���͸� ����� ���� ���ϸ� �ܻ�ó�� ����
        float3 offset = reflect(gSsaoCB.OffsetVectors[i].xyz, randVec);
        
        // ������ ���Ͱ� (p, n)���� ���ǵǴ� ����� ������ ���ϸ� �ݴ�� �����´�(�ݱ��� ����).
        float flip = sign(dot(offset, n));
        
        // p�� �߽����� ��������� �̳� ������ �� q�� �����Ѵ�.
        float3 q = p + flip * gSsaoCB.OcclusionRadius * offset;
        
        // q�� �����Ͽ� ���� �ؽ�ó ��ǥ�� ���Ѵ�.
        float4 projQ = mul(float4(q, 1.f), gSsaoCB.MtxProjTex);
        projQ /= projQ.w;
        
        // �������� q�� ���� ���� ����� �ȼ��� ���� ���� ���Ͽ� �� �������� ��ȯ
        float rz = gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, projQ.xy, 0.f).r;
        rz = NdcDepthToViewDepth(rz);
        
        // �þ� ���� ��ġ�� q�� ������ r ���͸� ���Ѵ�.
        float3 r = (rz / q.z) * q;
        
        // ���� r ���� p ���� �Ÿ��� ����� �۰ų� r - p�� n ������ ������ 90 ���ϸ� ���󵵿� �⿩�Ѵ�.
		float distZ = p.z - r.z;
        float dp = max(dot(n, normalize(r - p)), 0.0f);

        float occlusion = dp * OcclusionFunction(distZ);

		occlusionSum += occlusion;
    }
    
    occlusionSum /= gSampleCount;
    
    float access = 1.f - occlusionSum;
    
    // SSAO�� ������ ȿ���� ������ SSAO�� ��� ������Ų��.
    return saturate(access);
}