#include "stdafx.h"
#include "Script_Ceratoferox.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"


void Script_Ceratoferox::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Ceratoferox::AttackCallback, this), 34);

	SetDetectSound("Ceratoferox Detect");
	SetDeathSound("Ceratoferox Death");
}
