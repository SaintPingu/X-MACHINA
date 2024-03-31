#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_GroundObject.h"

#include "Scene.h"
#include "Object.h"



void Script_Enemy::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_GroundObject>();
	Transform* gunPos = mObject->FindFrame("WeaponAction");
	mGun = scene->Instantiate("SM_SciFiAssaultPistol");
	gunPos->SetChild(mGun);
	mObject->SetTag(ObjectTag::Enemy);
}

void Script_Enemy::Start()
{
	base::Start();
}

void Script_Enemy::Update()
{
	base::Update();

}

void Script_Enemy::OnDestroy()
{
	mGun->OnDestroy();
}
