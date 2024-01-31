#pragma once

#pragma region ClassForwardDecl
class LUTShader;
#pragma endregion

class LUTFilter : private UnCopyable {
private:
	UINT	mWidth{};
	UINT	mHeight{};
	float	mElapsedTime{};

	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_GPU_DESCRIPTOR_HANDLE mLUT0GpuSrv{};
	D3D12_GPU_DESCRIPTOR_HANDLE mLUT1GpuSrv{};
	D3D12_GPU_DESCRIPTOR_HANDLE mInputGpuSrv{};
	D3D12_GPU_DESCRIPTOR_HANDLE mOutputGpuUav{};

	ComPtr<ID3D12Resource> mInput{};
	ComPtr<ID3D12Resource> mOutput{};

	uptr<LUTShader> mLUTShader{};

public:
#pragma region C/Dtor
	LUTFilter(UINT width, UINT height, DXGI_FORMAT format);
	virtual ~LUTFilter() = default;
#pragma endregion

#pragma region Getter
	ID3D12Resource* Resource();
#pragma endregion

public:
	void Create();
	void Execute(ID3D12Resource* input);
	void CopyResource(ID3D12Resource* input);

private:
	void CreateDescriptors();
	void CreateResources();
}; 

