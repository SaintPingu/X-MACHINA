#include "stdafx.h"
#include "Script_LobbyManager.h"

#include "Script_LobbyUI.h"

#include "X-Engine.h"
#include "InputMgr.h"
#include "SoundMgr.h"
#include "Timer.h"
#include "LobbyScene.h"
#include "Light.h"
#include "Object.h"
#include "LobbyScene.h"
#include "GameFramework.h"
#include "Animator.h"
#include "AnimatorController.h"

#include "Component/Camera.h"
#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"


LobbyPlayer::LobbyPlayer(const LobbyPlayerInfo& info)
	: mInfo(info)
{
	mObject = LobbyScene::I->Instantiate("EliteTrooper");
}



void Script_LobbyManager::Awake()
{
	base::Awake();

	MainCamera::I->Awake();
	MAIN_CAMERA->SetOffset(Vec3(9.88f, 1.86f, 6.93f));
	MainCamera::I->SetPosition(Vec3(9.88f, 2.16f, 6.93f));
	MainCamera::I->SetLocalRotation(Vec3(15.25f, -124.f, 0.f));
	//MainCamera::I->LookAt({ 0, 1, 0 }, Vector3::Up);
	MainCamera::I->MoveForward(1.f);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 60.f);

	mObject->AddComponent<Script_LobbyUI>();
}

void Script_LobbyManager::Start()
{
	base::Start();

	const auto& trooper = LobbyScene::I->Instantiate("EliteTrooper");
	trooper->SetPosition(7.4f, 0, 5.27f);
	trooper->SetLocalRotation(Vec3(0.f, 53.41f, 0.f));

	LobbyScene::I->GetLight()->SetSunlightDir(Vec3(-0.3f, -0.6f, -0.6));
	LobbyScene::I->GetLight()->SetSunlightColor(Vector3::One * 0.52f);

	SoundMgr::I->Play("BGM", "Lobby", 1.0f, true);
}

void Script_LobbyManager::Update()
{
	base::Update();

	if (KEY_TAP('Q')) {
		auto cpkt = FBS_FACTORY->CPkt_PlayGame();
		CLIENT_NETWORK->Send(cpkt);
	}
}


void Script_LobbyManager::AddPlayer(const LobbyPlayerInfo& info)
{
	if (mLobbyPlayers.count(info.ID)) {
		return;
	}

	mLobbyPlayers[info.ID] = std::make_shared<LobbyPlayer>(info);
}

void Script_LobbyManager::RemovePlayer(UINT32 id)
{
	if (!mLobbyPlayers.count(id)) {
		return;
	}

	GameObject* target = mLobbyPlayers[id]->GetObj();
	mLobbyPlayers[id] = nullptr;
	LobbyScene::I->RemoveSkinMeshObject(target);
}

void Script_LobbyManager::ChangeToBattleScene()
{
	Engine::I->LoadScene(SceneType::Battle);
}
