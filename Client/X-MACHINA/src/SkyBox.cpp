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
	mMesh = std::make_shared<ModelObjectMesh>();
	mMesh->CreateSphereMesh(0.5f);
	mTexture = res->Get<Texture>("SkyBox_0");
}

void SkyBox::Render()
{
	res->Get<Shader>("SkyBox")->Set();
	mMesh->Render();
}