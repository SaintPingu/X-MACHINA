#include "stdafx.h"
#include "TestCube.h"

#include "Scene.h"

#include "ResourceMgr.h"
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"
#include "Model.h"

TestCube::TestCube(Vec2 pos) : Transform(this)
{
	SetPosition(Vec3(pos.x, scene->GetTerrainHeight(pos.x, pos.y) + size / 2.f, pos.y));

	mMesh = std::make_unique<ModelObjectMesh>();
	mMesh->CreateCubeMesh(size, size, size, true);
	
	MaterialLoadInfo materialInfo{};
	materialInfo.DiffuseAlbedo = Vec4(1.f, 1.f, 1.f, 1.f);

	sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(materialInfo);
	mMaterial = std::make_shared<Material>();
	mMaterial->SetMaterialColors(materialColors);
}

void TestCube::Render()
{
	mMaterial->UpdateShaderVars();
	UpdateShaderVars(0, mMaterial->mMatIndex);

	mMesh->Render();
}

