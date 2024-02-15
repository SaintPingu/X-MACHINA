#pragma once

#pragma region ClassForwardDecl
class Texture;
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
	D3D12_GPU_DESCRIPTOR_HANDLE mOutputGpuUav{};
	D3D12_GPU_DESCRIPTOR_HANDLE mOutputGpuSrv{};

	sptr<Texture> mOutput{};

	uptr<LUTShader> mLUTShader{};

public:
#pragma region C/Dtor
	LUTFilter(UINT width, UINT height, DXGI_FORMAT format);
	virtual ~LUTFilter() = default;
#pragma endregion

public:
	void Create();

	// ���͸� �����ϰ� ���� ��� �ؽ�ó �ε����� ��ȯ�Ѵ�.
	UINT Execute(rsptr<Texture> input);

private:
	void CreateDescriptors();
	void CreateResources();
}; 

