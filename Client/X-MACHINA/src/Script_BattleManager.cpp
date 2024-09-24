#include "stdafx.h"
#include "Script_BattleManager.h"

#include "Script_Ursacetus.h"
#include "Script_Onyscidus.h"
#include "Script_AdvancedCombatDroid_5.h"
#include "Script_Arack.h"
#include "Script_Ceratoferox.h"
#include "Script_Anglerox.h"
#include "Script_MiningMech.h"
#include "Script_Aranobot.h"
#include "Script_Gobbler.h"
#include "Script_Rapax.h"
#include "Script_LightBipedMech.h"

#include "Script_Player.h"
#include "Script_AbilityManager.h"
#include "Script_MainCamera.h"
#include "Script_Item.h"
#include "Script_PlayerController.h"

#include "Script_BattleUI.h"
#include "Script_StageNameUI.h"

#include "Script_SceneManager.h"
#include "Script_LobbyManager.h"

#include "ClientNetwork/Contents/Script_NetworkPlayer.h"
#include "ClientNetwork/Contents/Script_NetworkRemotePlayer.h"

#include "Component/ParticleSystem.h"
#include "Component/Camera.h"
#include "Component/Collider.h"

#include "GameFramework.h"

#include "BattleScene.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"
#include "ScriptExporter.h"
#include "ResourceMgr.h"
#include "SoundMgr.h"
#include "TextMgr.h"
#include "InputMgr.h"
#include "X-Engine.h"
#include "Component/UI.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"



void Script_BattleManager::Awake()
{
	base::Awake();

	InitComponents();
	InitPlayers();
	InitSceneObjectScripts();
	InitCustomObjectScripts();
}

void Script_BattleManager::Start()
{
	base::Start();

	mMainCamera = MainCamera::I->GetComponent<Script_MainCamera>();

	SoundMgr::I->Play("BGM", "Battle_Stage1", 1.0f, true);

	mObject->AddComponent<Script_StageNameUI>();

	ParticleManager::I->Play("Scene Dust", mPlayer);
}


void Script_BattleManager::Update()
{
	base::Update();
	
	if (KEY_TAP('Z')) {
		ResourceMgr::I->ReloadParticles();
	}
}

void Script_BattleManager::Reset()
{
	base::Reset();

	GameFramework::I->DisconnectServer();
	mMainCamera = nullptr;
	GameFramework::I->ResetPlayer();
}

void Script_BattleManager::InitComponents()
{
	MainCamera::I->AddComponent<Script_MainCamera>();
	mUI = mObject->AddComponent<Script_BattleUI>().get();
	mObject->AddComponent<Script_AbilityManager>();
}

void Script_BattleManager::InitPlayers()
{
	const auto& lobbyManager = Script_SceneManager::I->LobbyManager();
	const auto& lobbyPlayers = lobbyManager->GetPlayers();
	for (const auto& [id, lobbyPlayer] : lobbyPlayers) {
		GridObject* player{};

		if (id == GameFramework::I->GetMyPlayerID()) {
			player = BattleScene::I->Instantiate("EliteTrooper", ObjectTag::Player);
			mPlayer = player;
			GameFramework::I->SetPlayer(player);
			GameFramework::I->SetPlayerScript(player->AddComponent<Script_PlayerController>().get());
			player->AddComponent<Script_PheroPlayer>();

#ifdef SERVER_COMMUNICATION
			player->AddComponent<Script_NetworkPlayer>();
#endif
		}
		else {
			player = BattleScene::I->Instantiate("EliteTrooper");
			player->AddComponent<Script_NetworkRemotePlayer>();
			player->GetAnimator()->GetController()->SetRemotePlayer();
		}

		player->FindFrame("SK_EliteTrooper")->SetTexture(lobbyPlayer->GetMatIndex());
		player->SetName(lobbyPlayer->GetName());
		player->SetID(id);

		auto event_data = CLIENT_NETWORK->CreateEvent_Add_BattlePlayer(player);
		CLIENT_NETWORK->RegisterEvent(event_data);
	}
}

void Script_BattleManager::InitSceneObjectScripts()
{
	BattleScene::I->ProcessInitScriptOjbects(std::bind(&Script_BattleManager::ProcessSceneObjectScript, this, std::placeholders::_1));
}

void Script_BattleManager::InitCustomObjectScripts()
{
	// hard coding
}

void Script_BattleManager::ProcessSceneObjectScript(sptr<Object> object)
{
	const auto& exporter = object->GetComponent<ScriptExporter>();
	if (!exporter) {
		return;
	}

	switch (Hash(exporter->GetName())) {
	case Hash("WeaponCrate"):
		object->AddComponent<Script_Item_WeaponCrate>()->LoadData(exporter);
		break;
	case Hash("Bound"):
		WriteBounds(object);
		break;
#ifdef SERVER_COMMUNICATION
	case Hash("AdvancedCombatDroid"):
	case Hash("Onyscidus"):
	case Hash("Ursacetus"):
	case Hash("Arack"):
	case Hash("Ceratoferox"):
	case Hash("Anglerox"):
	case Hash("MiningMech"):
	case Hash("Aranobot"):
	case Hash("Gobbler"):
	case Hash("Rapax"):
	case Hash("LightBipedMech"):
		break;
#else
	case Hash("AdvancedCombatDroid"):
		object->AddComponent<Script_AdvancedCombatDroid_5>();
		break;
	case Hash("Onyscidus"):
		object->AddComponent<Script_Onyscidus>();
		break;
	case Hash("Ursacetus"):
		object->AddComponent<Script_Ursacetus>();
		break;
	case Hash("Arack"):
		object->AddComponent<Script_Arack>();
		break;
	case Hash("Ceratoferox"):
		object->AddComponent<Script_Ceratoferox>();
		break;
	case Hash("Anglerox"):
		object->AddComponent<Script_Anglerox>();
		break;
	case Hash("MiningMech"):
		object->AddComponent<Script_MiningMech>();
		break;
	case Hash("Aranobot"):
		object->AddComponent<Script_Aranobot>();
		break;
	case Hash("Gobbler"):
		object->AddComponent<Script_Gobbler>();
		break;
	case Hash("Rapax"):
		object->AddComponent<Script_Rapax>();
		break;
	case Hash("LightBipedMech"):
		object->AddComponent<Script_LightBipedMech>();
		break;
#endif
	default:
		throw std::runtime_error("[Error] Couldn't import script");
		break;
	}
}

void Script_BattleManager::WriteBounds(rsptr<Object> object)
{
	object->RemoveAllComponents<ObjectCollider>();
	object->RemoveAllComponents<BoxCollider>();
	object->RemoveAllComponents<SphereCollider>();
	const auto& exporter = object->GetComponent<ScriptExporter>();

	int boundingCnt{};
	exporter->GetData("Size", boundingCnt);

	// Sphere
	{
		const auto& collider = object->AddComponent<SphereCollider>();
		MyBoundingSphere bs;
		exporter->GetData("rad", bs.Radius);
		collider->SetBoundingSphere(bs);
		collider->Update();
	}

	// Box
	{
		for (int i = 0; i < boundingCnt; ++i) {
			const auto& collider = object->AddComponent<BoxCollider>();
			const std::string& idx = std::to_string(i);

			MyBoundingOrientedBox box;
			exporter->GetData(idx + "x", box.Extents.x);
			exporter->GetData(idx + "y", box.Extents.y);
			exporter->GetData(idx + "z", box.Extents.z);
			collider->SetBoundingBox(box);
			collider->Update();
		}
	}

	object->AddComponent<ObjectCollider>()->SetBoundColor(Vec3(1.f, 1.f, 0.f));
	std::static_pointer_cast<GridObject>(object)->ResetCollider();
}
