#include "stdafx.h"
#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "Script_LiveObject.h"

void Script_EnemyManager::Update()
{
	if (mTarget) {
		if (mTarget->GetComponent<Script_LiveObject>()->IsDead()) {
			mTarget = nullptr;
		}
	}
}

void Script_EnemyManager::RemoveAllAnimation()
{
	mController->SetValue("Walk", false);
	mController->SetValue("Attack", false);
	mController->SetValue("Return", false);
	mController->SetValue("GetHit", false);
}
