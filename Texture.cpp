#include "stdafx.h"
#include "Texture.h"
#include "DXGIMgr.h"

#include "Scene.h"

CTexture::CTexture(UINT nTextureType)
{
	mTextureType = nTextureType;
	mRootParamIndex = crntScene->GetRootParamIndex(RootParam::Texture);
}

CTexture::~CTexture()
{

}

void CTexture::SetName(const std::string& name)
{
	std::string str = name;
	mName.assign(str.begin(), str.end());
}

void CTexture::SetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	mSrvGpuDescHandle = d3dSrvGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables()
{
	if (mSrvGpuDescHandle.ptr) {
		cmdList->SetGraphicsRootDescriptorTable(mRootParamIndex, mSrvGpuDescHandle);
		crntScene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, mTextureMask, 32);
	}

}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::ReleaseUploadBuffers()
{
	mTextureUploadBuffer = nullptr;
}

ComPtr<ID3D12Resource> CTexture::CreateTexture(UINT width, UINT height, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS resourcecFlags, D3D12_RESOURCE_STATES resourceStates, D3D12_CLEAR_VALUE* clearValue)
{
	return mTexture = ::CreateTexture2DResource(width, height, 1, 0, dxgiFormat, resourcecFlags, resourceStates, clearValue);
}


void CTexture::LoadTexture(const std::wstring& filePath)
{
	::CreateTextureResourceFromDDSFile(filePath, mTextureUploadBuffer, mTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	crntScene->CreateShaderResourceView(this, 0);

	mTextureMask |= MATERIAL_ALBEDO_MAP;
}


void CTexture::LoadTexture(const std::string& textureName)
{
	SetName(textureName);
	std::wstring filePath = L"Models/Textures/" + mName + L".dds";
	LoadTexture(filePath);
}

void CTexture::LoadUITexture(const std::string& textureName)
{
	SetName(textureName);
	std::wstring filePath = L"Models/UI/" + mName + L".dds";
	LoadTexture(filePath);
}


void CTexture::LoadCubeTexture(const std::string& textureName)
{
	SetName(textureName);
	std::wstring filePath = L"Models/Skybox/" + mName + L".dds";
	LoadTexture(filePath);
}



D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc()
{
	ComPtr<ID3D12Resource> shaderResource = GetResource();
	D3D12_RESOURCE_DESC resourceDesc = shaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (mTextureType)
	{
	case RESOURCE_TEXTURE2D: //(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(resourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		shaderResourceViewDesc.Format = resourceDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = -1;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		shaderResourceViewDesc.Format = resourceDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.TextureCube.MipLevels = 1;
		shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		shaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	default:
		assert(0);
		break;
	}
	return shaderResourceViewDesc;
}