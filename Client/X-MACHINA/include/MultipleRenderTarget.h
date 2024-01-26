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

	// texture resource�� �����ϰ� �̿� ���� SRV�� RTV�� �����Ѵ�
	void CreateResourcesAndRtvsSrvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

	// �� RTV�� handle�� OutputMerger�� Set�Ѵ�.
	void OnPrepareRenderTargets(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle);
	
	// ������ �� ���ҽ��� ���¸� �����Ѵ�. (ResourceBarrier)
	void OnPostRenderTarget();

private:
	// texture resource�� �����Ѵ� (ID3D12Resource)
	void CreateTextureResources();
	// resource�� SRV Descriptor�� �����Ѵ�. (ID3D12Device::CreateShaderResourceView)
	void CreateSrvs();
	// resource�� RTV Descriptor�� �����Ѵ�. (ID3D12Device::CreateRenderTargetView)
	void CreateRtvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

};
#pragma endregion

