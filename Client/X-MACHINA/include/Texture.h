#pragma once

#pragma region Include
#include "Resources.h"
#pragma endregion

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
class Texture : public Resource, public std::enable_shared_from_this<Texture> {
private:
	D3DResource	mTextureType{};
	DWORD		mTextureMask{};

	ComPtr<ID3D12Resource> mTexture{};
	ComPtr<ID3D12Resource> mTextureUploadBuffer{};

	UINT mSrvDescriptorHandleIndex{};
	D3D12_GPU_DESCRIPTOR_HANDLE mSrvDescriptorHandle{};	// SRV�� �ڵ鰪 (�� descriptor heap ��ġ�� resource�� �ִ�)
	D3D12_GPU_DESCRIPTOR_HANDLE mUavDescriptorHandle{};	// UAV�� �ڵ鰪 (�� descriptor heap ��ġ�� resource�� �ִ�)
	
	UINT mRootParamIndex{};

public:
	Texture();
	virtual ~Texture() = default;

	ComPtr<ID3D12Resource> GetResource() const						{ return mTexture; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle() const		{ return mSrvDescriptorHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavGpuDescriptorHandle() const   { return mUavDescriptorHandle; }
	const UINT GetGpuDescriptorHandleIndex() const					{ return mSrvDescriptorHandleIndex; }

	float GetWidth() { return static_cast<float>(mTexture->GetDesc().Width); }
	float GetHeight() { return static_cast<float>(mTexture->GetDesc().Height); }

	// ���� resource�� ���� SRV_DESC�� ��ȯ�Ѵ�.
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc() const;
	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUnorderedAccessViewDesc() const;

	void SetTextureType(D3DResource textureType);

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
	virtual void Load(const std::string& name, const std::string& path) override;

	// ID3D12Device::CreateCommittedResource
	ComPtr<ID3D12Resource> Create(
		UINT					width,
		UINT					height,
		DXGI_FORMAT				dxgiFormat,
		D3D12_RESOURCE_FLAGS	resourcecFlags,
		D3D12_RESOURCE_STATES	resourceStates,
		Vec4					clearColor = Vec4());

	// �̹� ������ ���ҽ�(backBuffers, depthStencilBuffers)�κ��� �ؽ�ó�� �����ϴ� �Լ�
	ComPtr<ID3D12Resource> Create(
		ComPtr<ID3D12Resource>	resource);
};
#pragma endregion
