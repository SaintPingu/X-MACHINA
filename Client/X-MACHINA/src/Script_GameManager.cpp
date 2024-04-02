#include "stdafx.h"
#include "Script_GameManager.h"

#include "Script_Droid.h"
#include "Script_Aranobot.h"

#include "Scene.h"
#include "Object.h"



void Script_GameManager::Awake()
{
	{
		sptr<GridObject> enemy = scene->Instantiate("Droid_OII_DesertCamo");
		auto& script = enemy->AddComponent<Script_Droid>();
		enemy->SetPosition(95, 0, 105);
		script->SetMaxHP(10);
	}

	{
		sptr<GridObject> enemy = scene->Instantiate("Aranobot_Army");
		auto& script = enemy->AddComponent<Script_Aranobot>();
		enemy->SetPosition(100, 0, 105);
		script->SetMaxHP(10);
	}
}

void Script_GameManager::Start()
{
}

void Script_GameManager::Update()
{
}
