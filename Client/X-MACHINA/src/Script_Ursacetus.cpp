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

	mEnemyMgr->mController->FindMotionByName("2HandsSmashAttack")->AddCallback(std::bind(&Script_Ursacetus::SmashCallback, this), 65);
}

void Script_Ursacetus::SmashCallback()
{
	GameFramework::I->GetGameManager()->GetCamera()->StartShake(1.f, 0.001f);

	if (Vec3::Distance(mEnemyMgr->mTarget->GetPosition(), mObject->GetPosition()) < 5.f) {
		auto liveObject = mEnemyMgr->mTarget->GetComponent<Script_LiveObject>();
		if (liveObject) {
			if (liveObject->Hit(20.f, mObject)) {
				mEnemyMgr->mTarget = nullptr;
			}
		}
	}
}
