#include "stdafx.h"
#include "Texture.h"
#include "DXGIMgr.h"

#include "ResourceMgr.h"

Texture::Texture()
	: 
	Resource(ResourceType::Texture),
	mTextureType(D3DResource::Texture2D),
	mRootParamIndex(dxgi->GetGraphicsRootParamIndex(RootParam::Texture))
{

}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture::GetShaderResourceViewDesc() const
{
	ComPtr<ID3D12Resource> shaderResource = GetResource();
	D3D12_RESOURCE_DESC resourceDesc      = shaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (mTextureType)
	{
	case D3DResource::Texture2D: //(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(resourceDesc.DepthOrArraySize == 1)
	case D3DResource::Texture2D_Array:
		srvDesc.Format = resourceDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		break;
	case D3DResource::TextureCube: //(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(resourceDesc.DepthOrArraySize == 6)
		srvDesc.Format = resourceDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.f;
		break;
	default:
		assert(0);
		break;
	}
	return srvDesc;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC Texture::GetUnorderedAccessViewDesc() const
{
	ComPtr<ID3D12Resource> shaderResource = GetResource();
	D3D12_RESOURCE_DESC resourceDesc = shaderResource->GetDesc();

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = resourceDesc.Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	return uavDesc;
}

void Texture::SetTextureType(D3DResource textureType)
{
	mTextureType = textureType;

	switch (textureType)
	{
	case D3DResource::TextureCube:
		mRootParamIndex = dxgi->GetGraphicsRootParamIndex(RootParam::SkyBox);
		break;
	default:
		break;
	}
}

void Texture::ReleaseUploadBuffers()
{
	mTextureUploadBuffer = nullptr;
}

void Texture::UpdateShaderVars()
{
	// 스카이 박스 전용이고 나머지는 Scene에서 한 프레임 당 한 번씩만 연결한다.
	if (mSrvDescriptorHandle.ptr) {
		cmdList->SetGraphicsRootDescriptorTable(mRootParamIndex, mSrvDescriptorHandle);
	}
}

void Texture::ReleaseShaderVars()
{
}


void Texture::Load(const std::string& name, const std::string& path)
{
	std::string filePath = path + name + ".dds";
	std::wstring wfilePath;
	wfilePath.assign(filePath.begin(), filePath.end());

	D3DUtil::CreateTextureResourceFromDDSFile(wfilePath, mTextureUploadBuffer, mTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	dxgi->CreateShaderResourceView(this);

	mTextureMask |= MaterialMap::Albedo;
}

ComPtr<ID3D12Resource> Texture::Create(UINT width, UINT height, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS resourcecFlags, D3D12_RESOURCE_STATES resourceStates, Vec4 clearColor)
{
	return mTexture = D3DUtil::CreateTexture2DResource(width, height, 1, 0, dxgiFormat, resourcecFlags, resourceStates, clearColor);
}

ComPtr<ID3D12Resource> Texture::Create(ComPtr<ID3D12Resource> resource)
{
	return mTexture = resource;
}