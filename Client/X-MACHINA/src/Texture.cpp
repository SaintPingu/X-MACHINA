#include "stdafx.h"
#include "Texture.h"
#include "DXGIMgr.h"

#include "Scene.h"


Texture::Texture(D3DResource resourceType)
	:
	mResourceType(resourceType),
	mRootParamIndex(scene->GetRootParamIndex(RootParam::Texture))
{
	switch (resourceType)
	{
	case D3DResource::TextureCube:
		mRootParamIndex = scene->GetRootParamIndex(RootParam::SkyBox); // Textrue2D와 다른 루트 파라미터를 사용해야 함
		break;
	default:
		break;
	}
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


void Texture::LoadTexture(const std::string& folder, const std::string& fileName)
{
	mName = fileName;

	std::string filePath = folder + mName + ".dds";
	std::wstring wfilePath;
	wfilePath.assign(filePath.begin(), filePath.end());

	D3DUtil::CreateTextureResourceFromDDSFile(wfilePath, mTextureUploadBuffer, mTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	scene->CreateShaderResourceView(this);

	mTextureMask |= MaterialMap::Albedo;
}


ComPtr<ID3D12Resource> Texture::CreateTexture(UINT width, UINT height, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS resourcecFlags, D3D12_RESOURCE_STATES resourceStates, Vec4 clearColor)
{
	return mTexture = D3DUtil::CreateTexture2DResource(width, height, 1, 0, dxgiFormat, resourcecFlags, resourceStates, clearColor);
}

ComPtr<ID3D12Resource> Texture::CreateTextureFromResource(ComPtr<ID3D12Resource> resource)
{
	return mTexture = resource;
}