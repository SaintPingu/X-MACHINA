#pragma once

#pragma region ClassForwardDecl
class Texture;
#pragma endregion

#pragma region Class
class BlurFilter : private UnCopyable {
private:
	static constexpr UINT mMaxBlurRadius = 5;

	float	mSigma{};	// 값이 커질수록 주변 픽셀의 가중치가 커진다. 안바꾸는 편이 좋다.
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

	// 윈도우 화면 사이즈 크기가 달라지면 리소스와 서술자를 다시 생성해야 한다.
	void OnResize(UINT width, UINT height);
	// 블러 필터를 실행하는 함수.
	UINT Execute(rsptr<Texture> input, int blurCount);

private:
	void CreateDescriptors();
	void CreateResources();
};
#pragma endregion
