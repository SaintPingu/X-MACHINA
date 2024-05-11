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
    // 뷰 공간에서의 노말 값과 깊이 값
    float3 n = normalize(mul(gTextureMaps[gPassCB.RT1G_NormalIndex].SampleLevel(gsamPointClamp, pin.UV, 0.f), gPassCB.MtxView).xyz);
    float pz = gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, pin.UV, 0.f).r;
    pz = NdcDepthToViewDepth(pz);
    
    // 현재 처리중인 픽셀의 재구축 지점
    // p = t * v, pz = t * vz, t = pz / vz
    // p = (pz / vz) * v
    float3 p = (pz / pin.PosV.z) * pin.PosV;
    
    // 랜덤 벡터를 샘플링하여 [0, 1]의 값을 [-1, 1]로 매핑한다.
    // 랜덤 벡터 맵이 크기가 작기 때문에 UV 값에 4를 곱하여 샘플링이 골고루 되도록 한다.
    float3 randVec = 2.f * gTextureMaps[gSsaoCB.RandomVectorIndex].SampleLevel(gsamLinearWrap, 4.f * pin.UV, 0.f).rgb - 1.f;
    
	float occlusionSum = 0.0f;
    
    for (int i = 0; i < gSampleCount; ++i)
    {
        // 미리 생성한 골고루 분포된 오프셋 벡터를 하나의 무작위 벡터 기준으로 반사시킨다.
        // 아무튼 랜덤한 골고루 분포된 벡터를 만들기 위함 안하면 잔상처럼 생김
        float3 offset = reflect(gSsaoCB.OffsetVectors[i].xyz, randVec);
        
        // 오프셋 벡터가 (p, n)으로 정의되는 평면의 뒤쪽을 향하면 반대로 뒤집는다(반구를 위함).
        float flip = sign(dot(offset, n));
        
        // p를 중심으로 차폐반지름 이내 무작위 점 q를 선택한다.
        float3 q = p + flip * gSsaoCB.OcclusionRadius * offset;
        
        // q를 투영하여 투영 텍스처 좌표를 구한다.
        float4 projQ = mul(float4(q, 1.f), gSsaoCB.MtxProjTex);
        projQ /= projQ.w;
        
        // 시점에서 q를 향한 가장 가까운 픽셀의 깊이 값을 구하여 뷰 공간으로 변환
        float rz = gTextureMaps[gPassCB.DefaultDsIndex].SampleLevel(gsamDepthMap, projQ.xy, 0.f).r;
        rz = NdcDepthToViewDepth(rz);
        
        // 시야 공간 위치의 q를 지나는 r 벡터를 구한다.
        float3 r = (rz / q.z) * q;
        
        // 만약 r 값이 p 값의 거리가 충분히 작거나 r - p와 n 사이의 각도가 90 이하면 차폐도에 기여한다.
		float distZ = p.z - r.z;
        float dp = max(dot(n, normalize(r - p)), 0.0f);

        float occlusion = dp * OcclusionFunction(distZ);

		occlusionSum += occlusion;
    }
    
    occlusionSum /= gSampleCount;
    
    float access = 1.f - occlusionSum;
    
    // SSAO가 극적인 효과를 내도록 SSAO의 대비를 증가시킨다.
    return saturate(access);
}