#include "stdafx.h"
#include "Texture.h"
#include "DXGIMgr.h"

#include "Scene.h"


Texture::Texture(D3DResource resourceType) : mResourceType(resourceType)
{
	mRootParamIndex = scene->GetRootParamIndex(RootParam::Texture);
}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture::GetShaderResourceViewDesc() const
{
	ComPtr<ID3D12Resource> shaderResource = GetResource();
	D3D12_RESOURCE_DESC resourceDesc      = shaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (mResourceType)
	{
	case D3DResource::Texture2D: //(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(resourceDesc.DepthOrArraySize == 1)
	case D3DResource::Texture2D_Array:
		srvDesc.Format                        = resourceDesc.Format;
		srvDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels           = -1;
		srvDesc.Texture2D.MostDetailedMip     = 0;
		srvDesc.Texture2D.PlaneSlice          = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		break;
	case D3DResource::TextureCube: //(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(resourceDesc.DepthOrArraySize == 6)
		srvDesc.Format                          = resourceDesc.Format;
		srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels           = 1;
		srvDesc.TextureCube.MostDetailedMip     = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.f;
		break;
	default:
		assert(0);
		break;
	}
	return srvDesc;
}

void Texture::SetName(const std::string& name)
{
	std::string str = name;
	mName.assign(str.begin(), str.end());
}

void Texture::ReleaseUploadBuffers()
{
	mTextureUploadBuffer = nullptr;
}

void Texture::UpdateShaderVars()
{
	if (mSrvGpuDescHandle.ptr) {
		cmdList->SetGraphicsRootDescriptorTable(mRootParamIndex, mSrvGpuDescHandle);
		scene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, mTextureMask, 32);
	}

}

void Texture::ReleaseShaderVars()
{
}


void Texture::LoadTexture(const std::string& textureName)
{
	SetName(textureName);
	std::wstring filePath = L"Models/Textures/" + mName + L".dds";
	LoadTexture(filePath);
}

void Texture::LoadUITexture(const std::string& textureName)
{
	SetName(textureName);
	std::wstring filePath = L"Models/UI/" + mName + L".dds";
	LoadTexture(filePath);
}


void Texture::LoadCubeTexture(const std::string& textureName)
{
	SetName(textureName);
	std::wstring filePath = L"Models/Skybox/" + mName + L".dds";
	LoadTexture(filePath);
}


ComPtr<ID3D12Resource> Texture::CreateTexture(UINT width, UINT height, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS resourcecFlags, D3D12_RESOURCE_STATES resourceStates, D3D12_CLEAR_VALUE* clearValue)
{
	return mTexture = D3DUtil::CreateTexture2DResource(width, height, 1, 0, dxgiFormat, resourcecFlags, resourceStates, clearValue);
}


void Texture::LoadTexture(const std::wstring& filePath)
{
	D3DUtil::CreateTextureResourceFromDDSFile(filePath, mTextureUploadBuffer, mTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	scene->CreateShaderResourceView(this, 0);

	mTextureMask |= static_cast<WORD>(MaterialMap::Albedo);
}