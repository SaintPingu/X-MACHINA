#include "stdafx.h"
#include "SkyBox.h"
#include "Camera.h"
#include "Scene.h"
#include "DXGIMgr.h"

#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"


CSkyBox::CSkyBox()
{
	mMesh = std::make_unique<CSkyBoxMesh>(20, 20, 20);

	mTexture = std::make_unique<CTexture>(RESOURCE_TEXTURE_CUBE);
	mTexture->LoadCubeTexture("SkyBox_0");

	mShader = std::make_unique<CSkyBoxShader>();
	mShader->CreateShader();

	crntScene->CreateShaderResourceView(mTexture.get(), 0);
}

void CSkyBox::Render()
{
	Vec3 pos = mainCamera->GetPosition();
	SetPosition(pos.x, pos.y, pos.z);
	
	UpdateShaderVariable();
	mTexture->UpdateShaderVariables();
	mShader->Render();
	mMesh->Render();
}