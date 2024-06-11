#include "stdafx.h"
#include "Script_Ursacetus.h"

#include "Script_EnemyManager.h"
#include "Script_GameManager.h"
#include "Script_MainCamera.h"
#include "Script_PheroObject.h"

#include "GameFramework.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

void Script_Ursacetus::Awake()
{
	base::Awake();

	SetMaxHP(1000);

	mEnemyMgr->mRotationSpeed = 150.f;
	mEnemyMgr->mMoveSpeed = 3.5f;
	mEnemyMgr->mDetectionRange = 20.f;
	mEnemyMgr->mAttackRange = 7.5f;
	mEnemyMgr->mGetHitName = "GetHitFront";
	mEnemyMgr->mAttackName = "2HandsSmashAttack";

	mEnemyMgr->mController->FindMotionByName("2HandsSmashAttack")->AddCallback(std::bind(&Script_Ursacetus::SmashCallback, this), 65);

	mObject->GetComponent<Script_PheroObject>()->SetPheroObjectLevel(5);
}

void Script_Ursacetus::SmashCallback()
{
	GameFramework::I->GetGameManager()->GetCamera()->StartShake(1.f, 0.001f);
}
