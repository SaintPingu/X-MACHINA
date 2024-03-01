#pragma once

#pragma region Include
#include "Resources.h"
#pragma endregion

#pragma region Class
class Texture : public Resource {
private:
	D3DResource						mTextureType{};

	ComPtr<ID3D12Resource>			mResource{};
	ComPtr<ID3D12Resource>			mTextureUploadBuffer{};

	D3D12_GPU_DESCRIPTOR_HANDLE		mSrvDescriptorHandle{};	// SRV의 핸들값
	D3D12_GPU_DESCRIPTOR_HANDLE		mUavDescriptorHandle{};	// UAV의 핸들값

	UINT							mSrvDescriptorHandleIndex{};

public:
#pragma region C/Dtor
	Texture(D3DResource textureType = D3DResource::Texture2D);
	virtual ~Texture() = default;
#pragma endregion

#pragma region Getter
	ComPtr<ID3D12Resource>			 GetResource() const { return mResource; }
	D3D12_GPU_DESCRIPTOR_HANDLE		 GetGpuDescriptorHandle() const	{ return mSrvDescriptorHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE		 GetUavGpuDescriptorHandle() const { return mUavDescriptorHandle; }
	const UINT						 GetGpuDescriptorHandleIndex() const { return mSrvDescriptorHandleIndex; }
	D3D12_SHADER_RESOURCE_VIEW_DESC	 GetShaderResourceViewDesc() const;
	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUnorderedAccessViewDesc() const;

	float GetWidth() { return static_cast<float>(mResource->GetDesc().Width); }
	float GetHeight() { return static_cast<float>(mResource->GetDesc().Height); }
#pragma endregion

#pragma region Setter
	void SetSrvGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvGpuDescriptorHandle, UINT index);
	void SetUavGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE uavGpuDescriptorHandle);
#pragma endregion

public:
	void ReleaseUploadBuffers();

	// load texture from [fileName] in [folder] (file must be .dds extension)
	// create texture resource(ID3D12Device::CreateCommittedResource)
	// ex) LoadTexture("Textures/", "textureA");	(path = Textures/textureA.dds)
	void LoadTexture(const std::string& name, const std::string& path);

	// ID3D12Device::CreateCommittedResource
	ComPtr<ID3D12Resource> Create(
		UINT					width,
		UINT					height,
		DXGI_FORMAT				dxgiFormat,
		D3D12_RESOURCE_FLAGS	resourcecFlags,
		D3D12_RESOURCE_STATES	resourceStates,
		Vec4					clearColor = Vec4());

	// 이미 생성된 리소스(backBuffers, depthStencilBuffers)로부터 텍스처를 생성하는 함수
	ComPtr<ID3D12Resource> Create(
		ComPtr<ID3D12Resource>	resource);
};
#pragma endregion
