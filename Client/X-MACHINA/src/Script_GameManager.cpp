#include "stdafx.h"
#include "Script_GameManager.h"

#include "Script_Droid.h"
#include "Script_Aranobot.h"
#include "Script_Ursacetus.h"
#include "Script_MeleeBT.h"
#include "Component/ParticleSystem.h"

#include "Scene.h"
#include "Object.h"



void Script_GameManager::Awake()
{
	base::Awake();

	//for (int i = 0; i < 1; ++i)
	//{
	//	sptr<GridObject> enemy = Scene::I->Instantiate("Ursacetus", ObjectTag::Enemy);
	//	auto& script = enemy->AddComponent<Script_Ursacetus>();
	//	enemy->AddComponent<Script_MeleeBT>();
	//	enemy->SetPosition(70 + i * 3, 0, 270);
	//	script->SetMaxHP(200);
	//}
}

void Script_GameManager::Start()
{
	base::Start();
}

void Script_GameManager::Update()
{
	base::Update();
}
