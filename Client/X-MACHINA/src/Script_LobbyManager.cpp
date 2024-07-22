#include "stdafx.h"
#include "Script_LobbyManager.h"

#include "X-Engine.h"
#include "InputMgr.h"
#include "Scene.h"
#include "LobbyScene.h"
#include "Component/Camera.h"



void Script_LobbyManager::Awake()
{
	base::Awake();

	MainCamera::I->SetPosition(Vec3(1, 2, 2));
	MainCamera::I->LookAt({ 0, 1, 0 }, Vector3::Up);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 60.f);
}

void Script_LobbyManager::Start()
{
	base::Start();

	LobbyScene::I->Instantiate("EliteTrooper");
}

void Script_LobbyManager::Update()
{
	base::Update();

	if (KEY_TAP('Q')) {
		Engine::I->LoadScene(SceneType::Battle);
	}
}