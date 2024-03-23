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
	// ��� ���۷� ���� ���� ����ġ ����
	float weights[11] = { gBlurInfo.w0, gBlurInfo.w1, gBlurInfo.w2, 
						  gBlurInfo.w3, gBlurInfo.w4, gBlurInfo.w5, 
						  gBlurInfo.w6, gBlurInfo.w7, gBlurInfo.w8, 
						  gBlurInfo.w9, gBlurInfo.w10 };

	// gCache ����� N + 2 * gMaxBlurRadius�� ������ �����ڸ� �ȼ��鵵 
	// �� ���� �ȼ��� �����Ͽ� ����ġ�� ����ؾ� �ϱ� �����̴�. 
	// �ε��� �� ������ �������� �ش��ϱ� ������ ������ ũ�⸸ŭ ������� �Ѵ�.
	if(groupThreadID.x < gBlurInfo.gBlurRadius) // �ش� �ȼ��� ���� ������ �����ڸ��� �ִ� ���
	{
		// gInput �ε��� ���� ���� ���� �޸� �ȼ� ���� ���� ���� �ȼ��� �����Ͽ� �����Ѵ�.
		int x = max(dispatchThreadID.x - gBlurInfo.gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	if(groupThreadID.x >= N - gBlurInfo.gBlurRadius) // �ش� �ȼ��� ���� �������� �����ڸ��� �ִ� ���
	{
		// gInput �ε��� ���� ���� ���� �޸� �ȼ� ���� ���� ������ �ȼ��� �����Ͽ� �����Ѵ�.
		int x = min(dispatchThreadID.x + gBlurInfo.gBlurRadius, gInput.Length.x-1);
		gCache[groupThreadID.x + 2 * gBlurInfo.gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}

	// ��ȿ�� �ε����� ���� �ٷ� ���ø��Ͽ� ���� �޸𸮿� �����Ѵ�.
	gCache[groupThreadID.x + gBlurInfo.gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

	// ���� �޸𸮸� ����ϱ� ������ ��� ���� �޸𸮿� ������ ���������� Ȯ���ؾ� �Ѵ�.
	GroupMemoryBarrierWithGroupSync();
	
	// ����ġ�� ����Ͽ� ���� �ȼ� ������ �����Ѵ�.
	float4 blurColor = float4(0, 0, 0, 0);
	for(int i = -gBlurInfo.gBlurRadius; i <= gBlurInfo.gBlurRadius; ++i)
	{
		int k = groupThreadID.x + gBlurInfo.gBlurRadius + i;
		blurColor += weights[i + gBlurInfo.gBlurRadius] * gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}