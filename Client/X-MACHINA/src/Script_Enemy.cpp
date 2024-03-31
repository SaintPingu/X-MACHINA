#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_GroundObject.h"

#include "Scene.h"
#include "Object.h"



void Script_Enemy::Awake()
{
	mObject->AddComponent<Script_GroundObject>();
	Transform* gunPos = mObject->FindFrame("WeaponAction");
	sptr<GridObject> gun = scene->Instantiate("SM_SciFiAssaultPistol");
	gunPos->SetChild(gun);
	mObject->SetTag(ObjectTag::Enemy);
}

void Script_Enemy::Start()
{
}

void Script_Enemy::Update()
{
}
