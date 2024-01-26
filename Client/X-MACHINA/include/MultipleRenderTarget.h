#pragma once

#pragma region ClassForwardDecl
class Texture;
struct PassConstants;
#pragma endregion

#pragma region Class
class MultipleRenderTarget
{
private:
	std::vector<sptr<Texture>> mTextures{};
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> mRtvHandles{};

	UINT mRtvCnt{};
	const DXGI_FORMAT* mRtvFormats{};

public:
	MultipleRenderTarget();
	virtual ~MultipleRenderTarget() = default;

public:
	// set multiple render target textures of pass constants buffer
	void SetMRTTsPassConstants(PassConstants& passConstants);

	// texture resource를 생성하고 이에 대한 SRV와 RTV를 생성한다
	void CreateResourcesAndRtvsSrvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

	// 각 RTV의 handle을 OutputMerger에 Set한다.
	void OnPrepareRenderTargets(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle);
	
	// 렌더링 후 리소스의 상태를 전이한다. (ResourceBarrier)
	void OnPostRenderTarget();

private:
	// texture resource를 생성한다 (ID3D12Resource)
	void CreateTextureResources();
	// resource의 SRV Descriptor를 생성한다. (ID3D12Device::CreateShaderResourceView)
	void CreateSrvs();
	// resource의 RTV Descriptor를 생성한다. (ID3D12Device::CreateRenderTargetView)
	void CreateRtvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

};
#pragma endregion

