#include "stdafx.h"
#include "Script_Gobbler.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"


void Script_Gobbler::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.AttackAnimName)->AddCallback(std::bind(&Script_Gobbler::AttackCallback, this), 20);
}
	