#include "EnginePch.h"
#include "LobbyScene.h"

#include "BattleScene.h"
#include "Object.h"
#include "Model.h"
#include "Light.h"
#include "SkyBox.h"
#include "ResourceMgr.h"
#include "DXGIMgr.h"
#include "Shader.h"

#include "Component/Camera.h"
#include "Component/UI.h"


void LobbyScene::RenderBegin()
{
}

void LobbyScene::RenderShadow()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RESOURCE<Shader>("Shadow_SkinMesh")->Set();
	//RESOURCE<Shader>("Shadow_Global")->Set();

	for (const auto& object : mObjects) {
		object->Render();
	}
}

void LobbyScene::RenderDeferred()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RenderObjects();
}

void LobbyScene::RenderCustomDepth()
{
}

void LobbyScene::RenderForward()
{
	RESOURCE<Shader>("SkyBox")->Set();
	mSkyBox->Render();
}


void LobbyScene::RenderUI()
{
	Canvas::I->Render();
}

void LobbyScene::ApplyDynamicContext()
{
}

void LobbyScene::Update()
{
	//ParticleManager::I->Update();
	UpdateObjects();

	MainCamera::I->Update();
	MAIN_CAMERA->UpdateViewMtx();
	mLight->Update();
	Canvas::I->Update();

	UpdateShaderVars();

	mManager->Update();
}

void LobbyScene::Build()
{
	Scene::Build();
	std::cout << "Load Lobby Scene...";

	Start();
	Canvas::I->CreateUI(0, "Title", { -1100, 0 }, 800, 600);
	std::cout << "OK\n";
}

void LobbyScene::Release()
{
	Scene::Release();
	mObjects.clear();
}

GameObject* LobbyScene::Instantiate(const std::string& modelName, const Vec3& pos)
{
	const auto& model = RESOURCE<MasterModel>(modelName);
	if (!model) {
		return nullptr;
	}

	sptr<GameObject> instance = std::make_shared<GameObject>();
	instance->SetPosition(Vector3::Zero);
	instance->SetModel(model);

	mObjects.push_back(instance);

	return instance.get();
}

void LobbyScene::Start()
{
	mLight->BuildLights();
	MainCamera::I->SetActive(true);
	mManager->SetActive(true);
}

void LobbyScene::UpdateObjects()
{
	for (const auto& object : mObjects) {
		object->Update();
	}

	for (const auto& object : mObjects) {
		object->Animate();
	}

	for (const auto& object : mObjects) {
		object->LateUpdate();
	}
}

void LobbyScene::RenderObjects()
{
	RESOURCE<Shader>("SkinMesh")->Set();
	//RESOURCE<Shader>("Global")->Set();

	for (const auto& object : mObjects) {
		object->Render();
	}
}