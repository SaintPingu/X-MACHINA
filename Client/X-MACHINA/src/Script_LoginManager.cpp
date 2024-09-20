#include "stdafx.h"
#include "Script_LoginManager.h"

#include "Script_LoginUI.h"

#include "Component/UI.h"

#include "Scene.h"
#include "X-Engine.h"
#include "InputMgr.h"

void Script_LoginManager::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_LoginUI>();
}

void Script_LoginManager::Update()
{
	base::Update();

	if (KEY_TAP('Q')) {
		ChangeToLobbyScene();
	}
}

void Script_LoginManager::ChangeToLobbyScene()
{
	Engine::I->LoadScene(SceneType::Lobby);
}