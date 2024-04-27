#include "stdafx.h"
#include "Script_GameManager.h"

#include "Script_Droid.h"
#include "Script_Aranobot.h"
#include "Script_MeleeBT.h"
#include "Component/ParticleSystem.h"

#include "Scene.h"
#include "Object.h"



void Script_GameManager::Awake()
{
	base::Awake();

	for (int i = 0; i < 5; ++i)
	{
		sptr<GridObject> enemy = Scene::I->Instantiate("Aranobot_Army", ObjectTag::Enemy);
		auto& script = enemy->AddComponent<Script_Aranobot>();
		enemy->AddComponent<Script_MeleeBT>();
		enemy->SetPosition(90 + i * 5, 0, 90 + i * 10);
		script->SetMaxHP(2000);
	}
}

void Script_GameManager::Start()
{
	base::Start();
}

void Script_GameManager::Update()
{
	base::Update();
}
