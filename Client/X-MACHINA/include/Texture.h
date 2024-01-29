#pragma once

#pragma region EnumClass
enum class MaterialMap : DWORD {
	None         = 0x00,
	Albedo       = 0x01,
	Specular     = 0x02,
	Normal       = 0x04,
	Metalic      = 0x08,
	Emission     = 0x10,
	DetailAlbedo = 0x20,
	DetailNormal = 0x40,
};
#pragma endregion


#pragma region Class
// Texture�� resource(ID3D12Resource)�� �����Ѵ�.
class Texture {
private:
	D3DResource	mResourceType{};
	DWORD		mTextureMask{};

	ComPtr<ID3D12Resource> mTexture{};
	ComPtr<ID3D12Resource> mTextureUploadBuffer{};

	std::string mName{};

	UINT mSrvDescriptorHandleIndex{};
	D3D12_GPU_DESCRIPTOR_HANDLE mSrvDescriptorHandle{};	// SRV�� �ڵ鰪 (�� descriptor heap ��ġ�� resource�� �ִ�)
	
	UINT mRootParamIndex{};

public:
	Texture(D3DResource resourceType);
	virtual ~Texture() = default;

	const std::string& GetName() const							{ return mName; }
	ComPtr<ID3D12Resource> GetResource() const					{ return mTexture; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle() const  { return mSrvDescriptorHandle; }
	const UINT GetGpuDescriptorHandleIndex() const				{ return mSrvDescriptorHandleIndex; }

	// ���� resource�� ���� SRV_DESC�� ��ȯ�Ѵ�.
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc() const;

	void SetRootParamIndex(UINT index) { mRootParamIndex = index; }
	void SetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvGpuDescriptorHandle, UINT index) {
		mSrvDescriptorHandle = srvGpuDescriptorHandle; 
		mSrvDescriptorHandleIndex = index;
	}

public:
	void ReleaseUploadBuffers();

	void UpdateShaderVars();
	void ReleaseShaderVars();

	// load texture from [fileName] in [folder] (file must be .dds extension)
	// create texture resource(ID3D12Device::CreateCommittedResource)
	// ex) LoadTexture("Textures/", "textureA");	(path = Textures/textureA.dds)
	void LoadTexture(const std::string& folder, const std::string& fileName);

	// ID3D12Device::CreateCommittedResource
	ComPtr<ID3D12Resource> CreateTexture(
		UINT					width,
		UINT					height,
		DXGI_FORMAT				dxgiFormat,
		D3D12_RESOURCE_FLAGS	resourcecFlags,
		D3D12_RESOURCE_STATES	resourceStates,
		Vec4					clearColor = Vec4{ 1.f, 1.f, 1.f, 1.f });

	// �̹� ������ ���ҽ�(backBuffers, depthStencilBuffers)�κ��� �ؽ�ó�� �����ϴ� �Լ�
	ComPtr<ID3D12Resource> CreateTextureFromResource(
		ComPtr<ID3D12Resource>	resource);
};
#pragma endregion
