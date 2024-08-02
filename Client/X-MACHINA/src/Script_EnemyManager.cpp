#include "stdafx.h"
#include "Script_EnemyManager.h"

#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"

#include "Script_LiveObject.h"


void Script_EnemyManager::Awake()
{
	base::Awake();

	mController = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
}

void Script_EnemyManager::Update()
{
	base::Update();
}

void Script_EnemyManager::Reset()
{
	mState = EnemyState::Idle;
	mTarget = nullptr;
	
	while (!mPath.empty()) {
		mPath.pop();
	}

	RemoveAllAnimation();
}

void Script_EnemyManager::RemoveAllAnimation()
{
	mController->SetValue("Walk", false);
	mController->SetValue("Attack", false);
	mController->SetValue("Return", false);
	mController->SetValue("GetHit", false);
}
