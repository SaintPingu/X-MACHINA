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
#include "Component/ParticleSystem.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"


LobbyPlayer::LobbyPlayer(const LobbyPlayerInfo& info, unsigned char idx)
	: mInfo(info)
{
	// <pos, rot>
	static const std::array<std::pair<Vec3, float>, 3> kTransforms {
		std::make_pair(Vec3(3.33f, 0, 20.223f), 35.823f),
		std::make_pair(Vec3(4.021297f, 0, 18.21149f), 18.933f),
		std::make_pair(Vec3(1.720729f, 0, 19.2602f), 24.906f),
	};

	if (idx >= kTransforms.size()) {
		return;
	}

	const auto& transform = kTransforms[idx];
	mObject = LobbyScene::I->Instantiate("EliteTrooper", transform.first);
	mObject->SetLocalRotation(Vec3(0, transform.second, 0));
}



void Script_LobbyManager::Awake()
{
	base::Awake();

	MainCamera::I->Awake();
	MainCamera::I->SetPosition(Vec3(3.960061f, 1.980184f, 23.99316f));
	MainCamera::I->SetLocalRotation(Vec3(4.813f, 173.318, 0.f));
	MainCamera::I->MoveForward(1.f);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 70.f);

	mObject->AddComponent<Script_LobbyUI>();
}

void Script_LobbyManager::Start()
{
	base::Start();

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

	mLobbyPlayers[info.ID] = std::make_shared<LobbyPlayer>(info, mCurPlayerSize++);

	if (info.ID == GameFramework::I->GetMyPlayerID()) {
		ParticleManager::I->Play("Scene Dust", mLobbyPlayers[info.ID]->GetObj());
	}
}

void Script_LobbyManager::RemovePlayer(UINT32 id)
{
	if (!mLobbyPlayers.count(id)) {
		return;
	}

	GameObject* target = mLobbyPlayers[id]->GetObj();
	mLobbyPlayers[id] = nullptr;
	LobbyScene::I->RemoveSkinMeshObject(target);

	--mCurPlayerSize;
}

void Script_LobbyManager::ChangeToBattleScene()
{
	Engine::I->LoadScene(SceneType::Battle);
}
