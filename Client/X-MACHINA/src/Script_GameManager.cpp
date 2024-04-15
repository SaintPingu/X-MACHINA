#include "stdafx.h"
#include "Script_GameManager.h"

#include "Script_Droid.h"
#include "Script_Aranobot.h"
#include "Script_MeleeBT.h"
#include "ParticleSystem.h"

#include "Scene.h"
#include "Object.h"



void Script_GameManager::Awake()
{
	//{
	//	sptr<GridObject> enemy = scene->Instantiate("Droid_OII_DesertCamo");
	//	auto& script = enemy->AddComponent<Script_Droid>();
	//	enemy->AddComponent<Script_MeleeBT>();
	//	enemy->SetPosition(105, 0, 103);
	//	script->SetMaxHP(10);
	//}

	{
		sptr<GridObject> enemy = scene->Instantiate("Aranobot_Army");
		enemy->AddComponent<ParticleSystem>()->Load("light")->SetTarget("Aranobot_CalfFrontLeft");
		enemy->AddComponent<ParticleSystem>()->Load("light")->SetTarget("Aranobot_CalfFrontRight");
		auto& script = enemy->AddComponent<Script_Aranobot>();
		enemy->AddComponent<Script_MeleeBT>();
		enemy->SetPosition(100, 0, 105);
		script->SetMaxHP(20);
	}
}

void Script_GameManager::Start()
{
}

void Script_GameManager::Update()
{

}
