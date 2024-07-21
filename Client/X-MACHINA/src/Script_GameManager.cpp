#include "stdafx.h"
#include "Script_GameManager.h"

#include "Script_Ursacetus.h"
#include "Script_Onyscidus.h"
#include "Script_AdvancedCombatDroid_5.h"
#include "Script_Arack.h"
#include "Script_Ceratoferox.h"
#include "Script_Anglerox.h"
#include "Script_MiningMech.h"
#include "Script_Aranobot.h"

#include "Script_MainCamera.h"
#include "Script_Item.h"
#include "Component/ParticleSystem.h"
#include "Component/Camera.h"

#include "GameFramework.h"

#include "Scene.h"
#include "Object.h"
#include "ScriptExporter.h"



void Script_GameManager::Awake()
{
	base::Awake();

	InitSceneObjectScripts();
	InitCustomObjectScripts();
}

void Script_GameManager::Start()
{
	base::Start();

	mMainCamera = MainCamera::I->GetComponent<Script_MainCamera>();
}

void Script_GameManager::Update()
{
	base::Update();
}

void Script_GameManager::InitSceneObjectScripts()
{
	Scene::I->ProcessInitScriptOjbects(std::bind(&Script_GameManager::ProcessSceneObjectScript, this, std::placeholders::_1));
}

void Script_GameManager::InitCustomObjectScripts()
{
	// hard coding
}

void Script_GameManager::ProcessSceneObjectScript(sptr<Object> object)
{
	const auto& exporter = object->GetComponent<ScriptExporter>();
	if (!exporter) {
		return;
	}

	switch (Hash(exporter->GetName())) {
	case Hash("WeaponCrate"):
	case Hash("WeaponCrate2"):
		object->AddComponent<Script_Item_WeaponCrate>()->LoadData(exporter);
		break;
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
	default:
		throw std::runtime_error("[Error] Couldn't import script");
		break;
	}
}
