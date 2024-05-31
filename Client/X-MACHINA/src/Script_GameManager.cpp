#include "stdafx.h"
#include "Script_GameManager.h"

#include "Script_Ursacetus.h"
#include "Script_Onyscidus.h"
#include "Script_AdvancedCombatDroid_5.h"
#include "Script_MeleeBT.h"
#include "Script_MainCamera.h"
#include "Script_PheroObject.h"
#include "Component/ParticleSystem.h"
#include "Component/Camera.h"

#include "GameFramework.h"

#include "Scene.h"
#include "Object.h"



void Script_GameManager::Awake()
{
	base::Awake();

	{
		sptr<GridObject> enemy = Scene::I->Instantiate("Ursacetus", ObjectTag::Enemy);
		auto& script = enemy->AddComponent<Script_Ursacetus>();
		enemy->AddComponent<Script_MeleeBT>();
		enemy->SetPosition(160, 0, 280);
	}

	{
		std::vector<Vec3> positions{
			Vec3(90, 0, 250),
			Vec3(90, 0, 264),
			Vec3(73, 0, 274),
			Vec3(55, 0, 289),
			Vec3(75, 0, 228),
			Vec3(112, 0, 226),
			Vec3(104, 0, 221),
			Vec3(111, 0, 241),
			Vec3(109, 0, 253),
			Vec3(144, 0, 227),
			Vec3(153, 0, 223),
			Vec3(148, 0, 260),
			Vec3(131, 0, 260),
			Vec3(86, 0, 293),
		};

		for (const auto& pos : positions) {
			sptr<GridObject> enemy = Scene::I->Instantiate("Onyscidus", ObjectTag::Enemy);
			auto& script = enemy->AddComponent<Script_Onyscidus>();
			enemy->AddComponent<Script_MeleeBT>();
			enemy->SetPosition(pos);
			enemy->Rotate(0, rand() % 360, 0);
		}
	}

	{
		std::vector<Vec3> positions{
			Vec3(70, 0, 247),
			Vec3(65, 0, 241),
			Vec3(58, 0, 232),
			Vec3(70, 0, 298),
			Vec3(110, 0, 273),
			Vec3(90, 0, 229),
			Vec3(145, 0, 243),
		};

		for (const auto& pos : positions) {
			sptr<GridObject> enemy = Scene::I->Instantiate("AdvancedCombatDroid_5", ObjectTag::Enemy);
			auto& script = enemy->AddComponent<Script_AdvancedCombatDroid_5>();
			enemy->AddComponent<Script_MeleeBT>();
			enemy->AddComponent<Script_PheroObject>();
			enemy->SetPosition(pos);
			enemy->Rotate(0, rand() % 360, 0);
		}
	}
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

void Script_GameManager::InitObjectScripts()
{

}
