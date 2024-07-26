#include "stdafx.h"
#include "Script_LobbyManager.h"

#include "Script_LobbyUI.h"

#include "X-Engine.h"
#include "InputMgr.h"
#include "Timer.h"
#include "Scene.h"
#include "Light.h"
#include "Object.h"
#include "LobbyScene.h"
#include "GameFramework.h"

#include "Component/Camera.h"



void Script_LobbyManager::Awake()
{
	base::Awake();

	//MainCamera::I->SetPosition(Vec3(5, 3, 2));
	MAIN_CAMERA->SetOffset(Vec3(9.88f, 1.86f, 6.93f));
	MainCamera::I->SetPosition(Vec3(9.88f, 1.86f, 6.93f));
	MainCamera::I->SetLocalRotation(Quaternion::ToQuaternion(Vec3(8.25f, -124.f, 0.f)));
	MainCamera::I->LookAt({ 0, 1, 0 }, Vector3::Up);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 60.f);

	mObject->AddComponent<Script_LobbyUI>();
}

void Script_LobbyManager::Start()
{
	base::Start();

	const auto& trooper = LobbyScene::I->Instantiate("EliteTrooper");
	trooper->SetPosition(7.4f, 0, 5.27f);
	trooper->SetLocalRotation(Quaternion::ToQuaternion(Vec3(0.f, 53.41f, 0.f)));

	LobbyScene::I->GetLight()->SetSunlightDir(Vec3(-1, -2, -2));
}

void Script_LobbyManager::Update()
{
	base::Update();

	//if (KEY_TAP('Q')) {
		ChangeToBattleScene();
	//}
}

void Script_LobbyManager::Reset()
{
	base::Reset();

	mObject->RemoveComponent<Script_LobbyUI>();
}

void Script_LobbyManager::ChangeToBattleScene()
{
	Engine::I->LoadScene(SceneType::Battle);
}
