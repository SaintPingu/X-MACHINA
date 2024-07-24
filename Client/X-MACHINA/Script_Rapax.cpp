#include "stdafx.h"
#include "Script_Rapax.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"


void Script_Rapax::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Rapax::AttackCallback, this), 35);
}
