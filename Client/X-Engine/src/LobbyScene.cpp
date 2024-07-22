#include "EnginePch.h"
#include "LobbyScene.h"

#include "X-Engine.h"
#include "BattleScene.h"
#include "Object.h"
#include "Model.h"
#include "Light.h"
#include "SkyBox.h"
#include "InputMgr.h"
#include "TextMgr.h"
#include "ResourceMgr.h"
#include "DXGIMgr.h"
#include "Shader.h"
#include "Animator.h"
#include "AnimatorMotion.h"
#include "AnimatorLayer.h"
#include "AnimatorController.h"

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

	if (KEY_TAP('Q')) {
		Engine::I->LoadScene(SceneType::Battle);
	}
}

void LobbyScene::Build()
{
	Scene::Build();
	std::cout << "Load Lobby Scene...";
	
	const auto& model = RESOURCE<MasterModel>("EliteTrooper");

	sptr<GameObject> instance = std::make_shared<GameObject>();
	instance->SetPosition(Vector3::Zero);
	instance->SetModel(model);
	auto controller = instance->GetAnimator()->GetController();
	controller->SetValue("Weapon", 2);
	mObjects.push_back(instance);

	std::cout << "OK\n";

	Start();
	Canvas::I->CreateUI(0, "Title", { -1100, 0 }, 800, 600);
}

void LobbyScene::Release()
{
	Scene::Release();
	mObjects.clear();
}

void LobbyScene::Start()
{
	MainCamera::I->SetActive(true);
	MainCamera::I->SetPosition(Vec3(1, 2, 2));
	MainCamera::I->LookAt({ 0, 1, 0 }, Vector3::Up);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 60.f);
	mLight->BuildLights();
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