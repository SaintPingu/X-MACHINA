#pragma once

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

class CTexture
{
public:
	CTexture(UINT nResourceType);
	virtual ~CTexture();

private:
	UINT	mTextureType{};
	UINT	mTextureMask{};

	ComPtr<ID3D12Resource> mTexture{};
	ComPtr<ID3D12Resource> mTextureUploadBuffer{};

	std::wstring mName{};

	DXGI_FORMAT mBufferFormat{};

	D3D12_GPU_DESCRIPTOR_HANDLE mSrvGpuDescHandle{};

	int mRootParamIndex{ -1 };

	void LoadTexture(const std::wstring& filePath);

public:

	void SetName(const std::string& name);
	void SetName(const std::wstring& name) { mName = name; }
	const std::wstring& GetName() const { return mName; }

	void UpdateShaderVariables();
	void ReleaseShaderVariables();

	void LoadTexture(const std::string& textureName);
	void LoadUITexture(const std::string& textureName);
	void LoadCubeTexture(const std::string& textureName);

	void SetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);

	ComPtr<ID3D12Resource> GetResource() const { return mTexture; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle() { return mSrvGpuDescHandle; }

	UINT GetTextureType() { return(mTextureType); }
	DXGI_FORMAT GetBufferFormat() { return mBufferFormat; }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc();

	void ReleaseUploadBuffers();

	void SetRootParamIndex(int index) { mRootParamIndex = index; }


	ComPtr<ID3D12Resource> CreateTexture(
		UINT width,
		UINT height,
		DXGI_FORMAT dxgiFormat,
		D3D12_RESOURCE_FLAGS resourcecFlags,
		D3D12_RESOURCE_STATES resourceStates,
		D3D12_CLEAR_VALUE* clearValue);
};