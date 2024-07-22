#include "stdafx.h"
#include "Script_Ursacetus.h"

#include "Script_EnemyManager.h"
#include "Script_MainCamera.h"
#include "Script_PheroObject.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

#include "Component/Camera.h"

void Script_Ursacetus::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.AttackAnimName)->AddCallback(std::bind(&Script_Ursacetus::AttackCallback, this), 65);
}

void Script_Ursacetus::AttackCallback()
{
	base::AttackCallback();

	MainCamera::I->GetComponent<Script_MainCamera>()->StartShake(1.f, 0.001f);
}
