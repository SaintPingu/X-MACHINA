#include "stdafx.h"
#include "Script_LoginManager.h"

#include "Script_LoginUI.h"

#include "Component/UI.h"

#include "GameFramework.h"
#include "Scene.h"
#include "X-Engine.h"
#include "InputMgr.h"

void Script_LoginManager::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_LoginUI>();
}

void Script_LoginManager::Start()
{
	base::Start();
}

void Script_LoginManager::Update()
{
	base::Update();

	if (KEY_TAP('Q')) {
		ChangeToLobbyScene();
	}

	if (KEY_TAP(VK_RETURN)) {
		GameFramework::I->ConnectServer();
		ChangeToLobbyScene();
	}
}

void Script_LoginManager::ChangeToLobbyScene()
{
	Engine::I->LoadScene(SceneType::Lobby);
}