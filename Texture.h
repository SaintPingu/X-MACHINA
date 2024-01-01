#pragma once

#pragma region EnumClass
enum class MaterialMap : WORD {
	None = 0x00,
	Albedo = 0x01,
	Specular = 0x02,
	Normal = 0x04,
	Metalic = 0x08,
	Emission = 0x10,
	DetailAlbedo = 0x20,
	DetailNormal = 0x40,
};
#pragma endregion


#pragma region Class
class Texture {
private:
	D3DResource	mResourceType{};
	WORD		mTextureMask{};

	ComPtr<ID3D12Resource> mTexture{};
	ComPtr<ID3D12Resource> mTextureUploadBuffer{};

	std::wstring mName{};

	DXGI_FORMAT mBufferFormat{};

	D3D12_GPU_DESCRIPTOR_HANDLE mSrvGpuDescHandle{};

	int mRootParamIndex{ -1 };

public:
	Texture(D3DResource resourceType);
	virtual ~Texture() = default;

	const std::wstring& GetName() const { return mName; }
	DXGI_FORMAT GetBufferFormat() const { return mBufferFormat; }
	ComPtr<ID3D12Resource> GetResource() const { return mTexture; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle() const { return mSrvGpuDescHandle; }
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc() const;

	void SetName(const std::string& name);
	void SetName(const std::wstring& name) { mName = name; }
	void SetRootParamIndex(int index) { mRootParamIndex = index; }
	void SetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvGpuDescriptorHandle) { mSrvGpuDescHandle = srvGpuDescriptorHandle ; }

public:
	void ReleaseUploadBuffers();

	void UpdateShaderVars();
	void ReleaseShaderVars();

	void LoadTexture(const std::string& textureName);
	void LoadUITexture(const std::string& textureName);
	void LoadCubeTexture(const std::string& textureName);

	ComPtr<ID3D12Resource> CreateTexture(
		UINT					width,
		UINT					height,
		DXGI_FORMAT				dxgiFormat,
		D3D12_RESOURCE_FLAGS	resourcecFlags,
		D3D12_RESOURCE_STATES	resourceStates,
		D3D12_CLEAR_VALUE*		clearValue);

private:
	void LoadTexture(const std::wstring& filePath);
};
#pragma endregion
