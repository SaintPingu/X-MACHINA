#pragma once

#pragma region ClassForwardDecl
class HorzBlurShader;
class VertBlurShader;
#pragma endregion

// �� ���Ϳ� ���� ���ҽ��� �ĸ� ������ ũ�Ⱑ ���� ��쿡�� 
// ȭ�� �� �ؽ�ó�� �������� ���� �ʰ� �ĸ� ���۸� �����ͼ� ���ҽ��� �����Ͽ� ����� �� �ִ�.
// ���Ŀ� �ĸ� ���۸� �����ͼ� ����ϴ� ���� �ƴ� ȭ�� �� �ؽ�ó�� �ٿ� ���ø� �������� �ؾ� ȿ���� ����.
// �ĸ� ������ 1/4 ũ��� �ٿ� ���ø��ϸ� �帱 �ȼ��� ������ �۾����� ȭ�� ��ü�� �������Ǹ� �ٽ� ������� �����̴�.
// �߰��� ������ ������ �̿��ؼ� ������Ʈ�� �����ϸ� notify ȣ��� ���͸� �����ų �����̴�.
#pragma region Class
class BlurFilter : private UnCopyable {
private:
	static constexpr UINT mMaxBlurRadius = 5;

	float	mSigma{};	// ���� Ŀ������ �ֺ� �ȼ��� ����ġ�� Ŀ����. �ȹٲٴ� ���� ����.
	UINT	mWidth{};
	UINT	mHeight{};

	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_GPU_DESCRIPTOR_HANDLE mBlur0GpuSrv{};
	D3D12_GPU_DESCRIPTOR_HANDLE mBlur0GpuUav{};
										
	D3D12_GPU_DESCRIPTOR_HANDLE mBlur1GpuSrv{};
	D3D12_GPU_DESCRIPTOR_HANDLE mBlur1GpuUav{};

	// ping-pong the textures
	ComPtr<ID3D12Resource> mBlurMap0{};
	ComPtr<ID3D12Resource> mBlurMap1{};

	uptr<HorzBlurShader> mHorzBlurShader{};
	uptr<VertBlurShader> mVertBlurShader{};

public:
#pragma region C/Dtor
	BlurFilter(UINT width, UINT height, DXGI_FORMAT format);
	virtual ~BlurFilter() = default;
#pragma endregion

#pragma region Getter
	ID3D12Resource* Resource();
#pragma endregion

public:
	void Create();

	// ������ ȭ�� ������ ũ�Ⱑ �޶����� ���ҽ��� �����ڸ� �ٽ� �����ؾ� �Ѵ�.
	void OnResize(UINT width, UINT height);
	// �� ���͸� �����ϴ� �Լ�.
	void Execute(ID3D12Resource* input, int blurCount);
	// ���� mBlurMap0 ���ҽ��� �ĸ� ���ۿ� �����ϴ� �Լ�
	void CopyResource(ID3D12Resource* input);

private:
	// �ֺ� �ȼ��� ����ġ�� ���ϱ� ���� �Լ��� ����ġ�� ���� 1�̴�.
	std::vector<float> CalculateGaussWeights(float sigma);

private:
	void CreateDescriptors();
	void CreateResources();
};
#pragma endregion
