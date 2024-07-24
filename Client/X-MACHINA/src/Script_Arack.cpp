#include "stdafx.h"
#include "Script_Arack.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"


void Script_Arack::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Arack::AttackCallback, this), 13);
}
