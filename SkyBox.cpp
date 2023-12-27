#include "stdafx.h"
#include "SkyBox.h"
#include "Camera.h"
#include "Scene.h"
#include "DXGIMgr.h"

#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"


SkyBox::SkyBox()
{
	mMesh = std::make_unique<SkyBoxMesh>(20, 20, 20);

	mTexture = std::make_unique<Texture>(RESOURCE_TEXTURE_CUBE);
	mTexture->LoadCubeTexture("SkyBox_0");

	mShader = std::make_unique<SkyBoxShader>();
	mShader->CreateShader();

	crntScene->CreateShaderResourceView(mTexture.get(), 0);
}

void SkyBox::Render()
{
	Vec3 pos = mainCamera->GetPosition();
	SetPosition(pos.x, pos.y, pos.z);
	
	UpdateShaderVariable();
	mTexture->UpdateShaderVariables();
	mShader->Render();
	mMesh->Render();
}