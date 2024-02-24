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
// Texture를 resource(ID3D12Resource)로 관리한다.
class Texture {
private:
	D3DResource	mResourceType{};
	DWORD		mTextureMask{};

	ComPtr<ID3D12Resource> mTexture{};
	ComPtr<ID3D12Resource> mTextureUploadBuffer{};

	std::string mName{};

	UINT mSrvDescriptorHandleIndex{};
	D3D12_GPU_DESCRIPTOR_HANDLE mSrvDescriptorHandle{};	// SRV의 핸들값 (이 descriptor heap 위치에 resource가 있다)
	D3D12_GPU_DESCRIPTOR_HANDLE mUavDescriptorHandle{};	// UAV의 핸들값 (이 descriptor heap 위치에 resource가 있다)
	
	UINT mRootParamIndex{};

public:
	Texture(D3DResource resourceType);
	virtual ~Texture() = default;

	const std::string& GetName() const								{ return mName; }
	ComPtr<ID3D12Resource> GetResource() const						{ return mTexture; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle() const		{ return mSrvDescriptorHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavGpuDescriptorHandle() const   { return mUavDescriptorHandle; }
	const UINT GetGpuDescriptorHandleIndex() const					{ return mSrvDescriptorHandleIndex; }

	float GetWidth() { return static_cast<float>(mTexture->GetDesc().Width); }
	float GetHeight() { return static_cast<float>(mTexture->GetDesc().Height); }

	// 현재 resource에 따른 SRV_DESC을 반환한다.
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc() const;
	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUnorderedAccessViewDesc() const;

	void SetRootParamIndex(UINT index) { mRootParamIndex = index; }
	void SetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvGpuDescriptorHandle, UINT index) {
		mSrvDescriptorHandle = srvGpuDescriptorHandle; 
		mSrvDescriptorHandleIndex = index;
	}
	void SetUavGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE uavGpuDescriptorHandle) { mUavDescriptorHandle = uavGpuDescriptorHandle; }

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
		Vec4					clearColor = Vec4{ 0.f, 0.f, 0.f, 0.f });

	// 이미 생성된 리소스(backBuffers, depthStencilBuffers)로부터 텍스처를 생성하는 함수
	ComPtr<ID3D12Resource> CreateTextureFromResource(
		ComPtr<ID3D12Resource>	resource);
};
#pragma endregion
