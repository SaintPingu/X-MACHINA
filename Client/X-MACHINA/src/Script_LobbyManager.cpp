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
#include "ResourceMgr.h"
#include "Texture.h"

#include "Component/Camera.h"
#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"

#include "Component/ParticleSystem.h"

LobbyPlayer::LobbyPlayer(const LobbyPlayerInfo& info)
	: mInfo(info)
{
	mObject = LobbyScene::I->Instantiate("EliteTrooper");
}



void Script_LobbyManager::Awake()
{
	base::Awake();

	MainCamera::I->Awake();
	MainCamera::I->SetPosition(Vec3(3.960061f, 1.980184f, 23.99316f));
	MainCamera::I->SetLocalRotation(Vec3(4.813f, 173.318f, 0.f));
	//MainCamera::I->LookAt({ 0, 1, 0 }, Vector3::Up);
	MainCamera::I->MoveForward(1.f);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 70.f);

	mObject->AddComponent<Script_LobbyUI>();
}

void Script_LobbyManager::Start()
{
	base::Start();

	{
		const auto& trooper = LobbyScene::I->Instantiate("EliteTrooper");
		trooper->SetPosition(3.33f, 0, 20.223f);
		trooper->SetLocalRotation(Vec3(0.f, 35.823f, 0.f));
		trooper->FindFrame("SK_EliteTrooper")->SetTexture(RESOURCE<Texture>("T_EliteTrooper_ForestCamo_BaseColor"));
		ParticleManager::I->Play("Scene Dust", trooper);
	}

	{
		const auto& trooper = LobbyScene::I->Instantiate("EliteTrooper");
		trooper->SetPosition(4.021297f, 0, 18.21149f);
		trooper->SetLocalRotation(Vec3(0.f, 18.933f, 0.f));
	}

	{
		const auto& trooper = LobbyScene::I->Instantiate("EliteTrooper");
		trooper->SetPosition(1.720729f, 0, 19.2602f);
		trooper->SetLocalRotation(Vec3(0.f, 24.906f, 0.f));
	}

	LobbyScene::I->GetLight()->SetSunlightDir(Vec3(-0.3f, -0.6f, -0.6));
	LobbyScene::I->GetLight()->SetSunlightColor(Vector3::One * 0.52f);

	SoundMgr::I->Play("BGM", "Lobby", 1.0f, true);
}

void Script_LobbyManager::Update()
{
	base::Update();
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
