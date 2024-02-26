#include "stdafx.h"
#include "SkyBox.h"
#include "DXGIMgr.h"

#include "Camera.h"

#include "ResourceMgr.h"
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"


SkyBox::SkyBox() : Transform(this)
{
	mMesh = std::make_unique<ModelObjectMesh>();
	mMesh->CreateSphereMesh(0.5f);

	mTexture = res->Load<Texture>("SkyBox_0", "Import/Skybox/");
	mTexture->SetTextureType(D3DResource::TextureCube);

	dxgi->CreateShaderResourceView(mTexture.get());
}

void SkyBox::SetGraphicsRootDescriptorTable()
{
	mTexture->UpdateShaderVars();
}

void SkyBox::Render()
{
	res->Get<Shader>("SkyBox")->Set();
	mMesh->Render();
}