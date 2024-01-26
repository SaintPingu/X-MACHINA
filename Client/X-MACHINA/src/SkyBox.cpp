#include "stdafx.h"
#include "SkyBox.h"

#include "Camera.h"
#include "Scene.h"

#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"


SkyBox::SkyBox() : Transform(this)
{
	mMesh = std::make_unique<ModelObjectMesh>();
	mMesh->CreateSkyBoxMesh(20, 20, 20);

	mTexture = std::make_unique<Texture>(D3DResource::TextureCube);
	mTexture->LoadTexture("Models/Skybox/", "SkyBox_0");

	mShader = std::make_unique<SkyBoxShader>();
	mShader->Create();

	scene->CreateShaderResourceView(mTexture.get(), 0);
}

void SkyBox::Render()
{
	Vec3 pos = mainCameraObject->GetPosition();
	SetPosition(pos.x, pos.y, pos.z);
	
	// 오직 스카이 박스 텍스처만 해당 함수를 사용해야 한다. 
	// 나머지 다른 텍스처들은 모두 씬의 Update에서 한 번만 설정하기 때문이다.
	mTexture->UpdateShaderVars();
	UpdateShaderVars();

	mShader->Set();
	mMesh->Render();
}