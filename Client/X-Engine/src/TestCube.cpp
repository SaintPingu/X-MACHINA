#include "EnginePch.h"
#include "TestCube.h"

#include "BattleScene.h"

#include "ResourceMgr.h"
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"
#include "Model.h"

TestCube::TestCube(Vec2 pos)
{
	SetPosition(Vec3(pos.x, BattleScene::I->GetTerrainHeight(pos.x, pos.y) + mSize / 2.f, pos.y));

	mMesh = std::make_unique<ModelObjectMesh>();
	mMesh->CreateSphere(mSize, 64, 64);
	
	MaterialLoadInfo materialInfo{};
	materialInfo.DiffuseAlbedo = Vec4(1.f, 1.f, 1.f, 1.f);

	sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(materialInfo);
	mMaterial = std::make_shared<Material>();
	mMaterial->SetMaterialColors(materialColors);

	mObjectCB.UseRefract = true;
}

void TestCube::Render()
{
	mMaterial->UpdateShaderVars();
	UpdateShaderVars(0, mMaterial->mMatIndex);

	mMesh->Render();
}

