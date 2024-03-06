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