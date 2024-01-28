#define N 256
#define CacheSize (N + 2 * gMaxBlurRadius)

static const int gMaxBlurRadius = 5;

struct BlurInfo
{
	int gBlurRadius;

	float w0;
	float w1;
	float w2;
	float w3;
	float w4;
	float w5;
	float w6;
	float w7;
	float w8;
	float w9;
	float w10;
};

ConstantBuffer<BlurInfo> gBlurInfo : register(b0);
Texture2D gInput				  : register(t0);
RWTexture2D<float4> gOutput		  : register(u0);
groupshared float4 gCache[CacheSize];

[numthreads(N, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	// 상수 버퍼로 전달 받은 가중치 값들
	float weights[11] = { gBlurInfo.w0, gBlurInfo.w1, gBlurInfo.w2, 
						  gBlurInfo.w3, gBlurInfo.w4, gBlurInfo.w5, 
						  gBlurInfo.w6, gBlurInfo.w7, gBlurInfo.w8, 
						  gBlurInfo.w9, gBlurInfo.w10 };

	// gCache 사이즈가 N + 2 * gMaxBlurRadius인 이유는 가장자리 픽셀들도 
	// 양 옆의 픽셀을 참고하여 가중치를 계산해야 하기 때문이다. 
	// 인덱스 외 범위는 검은색에 해당하기 때문에 지름의 크기만큼 더해줘야 한다.
	if(groupThreadID.x < gBlurInfo.gBlurRadius) // 해당 픽셀이 가장 왼쪽의 가장자리에 있는 경우
	{
		// gInput 인덱스 범위 외의 공유 메모리 픽셀 값을 가장 왼쪽 픽셀로 한정하여 저장한다.
		int x = max(dispatchThreadID.x - gBlurInfo.gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	if(groupThreadID.x >= N - gBlurInfo.gBlurRadius) // 해당 픽셀이 가장 오른쪽의 가장자리에 있는 경우
	{
		// gInput 인덱스 범위 외의 공유 메모리 픽셀 값을 가장 오른쪽 픽셀로 한정하여 저장한다.
		int x = min(dispatchThreadID.x + gBlurInfo.gBlurRadius, gInput.Length.x-1);
		gCache[groupThreadID.x + 2 * gBlurInfo.gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}

	// 유효한 인덱스인 경우는 바로 샘플링하여 공유 메모리에 저장한다.
	gCache[groupThreadID.x + gBlurInfo.gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

	// 공유 메모리를 사용하기 때문에 모든 공유 메모리에 저장이 끝났는지를 확인해야 한다.
	GroupMemoryBarrierWithGroupSync();
	
	// 가중치를 계산하여 최종 픽셀 색상을 결정한다.
	float4 blurColor = float4(0, 0, 0, 0);
	for(int i = -gBlurInfo.gBlurRadius; i <= gBlurInfo.gBlurRadius; ++i)
	{
		int k = groupThreadID.x + gBlurInfo.gBlurRadius + i;
		blurColor += weights[i + gBlurInfo.gBlurRadius] * gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	float weights[11] = { gBlurInfo.w0, gBlurInfo.w1, gBlurInfo.w2, 
						  gBlurInfo.w3, gBlurInfo.w4, gBlurInfo.w5, 
						  gBlurInfo.w6, gBlurInfo.w7, gBlurInfo.w8, 
						  gBlurInfo.w9, gBlurInfo.w10 };

	if(groupThreadID.y < gBlurInfo.gBlurRadius)
	{
		int y = max(dispatchThreadID.y - gBlurInfo.gBlurRadius, 0);
		gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
	}
	if(groupThreadID.y >= N - gBlurInfo.gBlurRadius)
	{
		int y = min(dispatchThreadID.y + gBlurInfo.gBlurRadius, gInput.Length.y-1);
		gCache[groupThreadID.y + 2 * gBlurInfo.gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
	}
	
	gCache[groupThreadID.y + gBlurInfo.gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

	GroupMemoryBarrierWithGroupSync();

	float4 blurColor = float4(0, 0, 0, 0);
	for(int i = -gBlurInfo.gBlurRadius; i <= gBlurInfo.gBlurRadius; ++i)
	{
		int k = groupThreadID.y + gBlurInfo.gBlurRadius + i;
		blurColor += weights[i + gBlurInfo.gBlurRadius] * gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}