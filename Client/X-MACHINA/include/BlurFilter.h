#pragma once

#pragma region ClassForwardDecl
class Texture;
class HorzBlurShader;
class VertBlurShader;
#pragma endregion

// 블러 필터와 같이 리소스와 후면 버퍼의 크기가 같을 경우에는 
// 화면 밖 텍스처에 렌더링을 하지 않고 후면 버퍼를 가져와서 리소스에 복사하여 사용할 수 있다.
// 추후에 후면 버퍼를 가져와서 사용하는 것이 아닌 화면 밖 텍스처로 다운 샘플링 렌더링을 해야 효율이 좋다.
// 후면 버퍼의 1/4 크기로 다운 샘플링하면 흐릴 픽셀의 개수도 작아지고 화면 전체에 렌더링되며 다시 흐려지기 때문이다.
// 추가로 옵저버 패턴을 이용해서 오브젝트를 관찰하며 notify 호출시 필터를 실행시킬 예정이다.
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

	uptr<HorzBlurShader> mHorzBlurShader{};
	uptr<VertBlurShader> mVertBlurShader{};

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
	// 주변 픽셀의 가중치를 구하기 위한 함수로 가중치의 합은 1이다.
	std::vector<float> CalculateGaussWeights(float sigma);

private:
	void CreateDescriptors();
	void CreateResources();
};
#pragma endregion
