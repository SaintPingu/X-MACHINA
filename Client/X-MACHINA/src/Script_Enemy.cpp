#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_EnemyManager.h"
#include "Script_GroundObject.h"
#include "Script_MeleeBT.h"

#include "X-Engine.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"



void Script_Enemy::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_GroundObject>();
	mObject->SetTag(ObjectTag::Enemy);

	mEnemyMgr = mObject->AddComponent<Script_EnemyManager>();
	mEnemyMgr->mController = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
}
