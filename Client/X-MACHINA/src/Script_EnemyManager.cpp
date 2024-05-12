#include "stdafx.h"
#include "Script_EnemyManager.h"

#include "AnimatorController.h"

void Script_EnemyManager::RemoveAllAnimation()
{
	mController->SetValue("Walk", false);
	mController->SetValue("Attack", false);
	mController->SetValue("Return", false);
	mController->SetValue("GetHit", false);
}
