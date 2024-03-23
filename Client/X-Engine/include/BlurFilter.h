#pragma once

#pragma region ClassForwardDecl
class Texture;
#pragma endregion

#pragma region Class
class BlurFilter : private UnCopyable {
private:
	static constexpr UINT mMaxBlurRadius = 5;

	float	mSigma{};	// ���� Ŀ������ �ֺ� �ȼ��� ����ġ�� Ŀ����. �ȹٲٴ� ���� ����.
	UINT	mWidth{};
	UINT	mHeight{};

	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_GPU_DESCRIPTOR_HANDLE mOutputGpuSrv{};
	D3D12_GPU_DESCRIPTOR_HANDLE mOutputGpuUav{};
										
	// ping-pong the textures
	sptr<Texture> mOutput{};

public:
#pragma region C/Dtor
	BlurFilter(UINT width, UINT height, DXGI_FORMAT format);
	virtual ~BlurFilter() = default;
#pragma endregion

public:
	void Create();

	// ������ ȭ�� ������ ũ�Ⱑ �޶����� ���ҽ��� �����ڸ� �ٽ� �����ؾ� �Ѵ�.
	void OnResize(UINT width, UINT height);
	// �� ���͸� �����ϴ� �Լ�.
	UINT Execute(rsptr<Texture> input, int blurCount);

private:
	void CreateDescriptors();
	void CreateResources();
};
#pragma endregion
