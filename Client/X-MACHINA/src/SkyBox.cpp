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
	mMesh->CreateSphereMesh(0.5f);

	mTexture = std::make_unique<Texture>(D3DResource::TextureCube);
	mTexture->LoadTexture("Import/Skybox/", "SkyBox_0");

	mShader = std::make_unique<SkyBoxShader>();
	mShader->Create(ShaderType::OffScreen);

	scene->CreateShaderResourceView(mTexture.get());
}

void SkyBox::SetGraphicsRootDescriptorTable()
{
	mTexture->UpdateShaderVars();
}

void SkyBox::Render()
{
	mShader->Set();
	mMesh->Render();
}