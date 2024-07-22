#include "stdafx.h"
#include "Script_Aranobot.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"


void Script_Aranobot::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.AttackAnimName)->AddCallback(std::bind(&Script_Aranobot::AttackCallback, this), 6);
}
