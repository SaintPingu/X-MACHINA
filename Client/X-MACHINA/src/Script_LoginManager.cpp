#include "stdafx.h"
#include "Script_LoginManager.h"

#include "Script_LoginUI.h"

#include "Component/UI.h"

#include "GameFramework.h"
#include "Scene.h"
#include "X-Engine.h"
#include "InputMgr.h"
#include "TextMgr.h"

void Script_LoginManager::Awake()
{
	base::Awake();

	mLoginUI = mObject->AddComponent<Script_LoginUI>();
}

void Script_LoginManager::Start()
{
	base::Start();

	GameFramework::I->ConnectServer();
}

void Script_LoginManager::Update()
{
	base::Update();

	if (GameFramework::I->IsLogin()) {
		ChangeToLobbyScene();
	}
}

void Script_LoginManager::FailLogin()
{
	mLoginUI->FailLogin();
}

void Script_LoginManager::ChangeToLobbyScene()
{
	Engine::I->LoadScene(SceneType::Lobby);
}