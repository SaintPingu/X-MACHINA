#include "stdafx.h"
#include "Script_GameManager.h"

#include "Script_Enemy.h"

#include "Scene.h"
#include "Object.h"



void Script_GameManager::Awake()
{
	sptr<GridObject> enemy = scene->Instantiate("Droid_OII_DesertCamo");
	enemy->AddComponent<Script_Enemy>();
	enemy->SetPosition(95, 0, 105);
	mEnemies.push_back(enemy);
}

void Script_GameManager::Start()
{
}

void Script_GameManager::Update()
{
}
