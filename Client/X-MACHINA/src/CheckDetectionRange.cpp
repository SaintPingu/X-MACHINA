#include "stdafx.h"
#include "CheckDetectionRange.h"

#include "Script_EnemyManager.h"

#include "X-Engine.h"
#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


CheckDetectionRange::CheckDetectionRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}


BT::NodeState CheckDetectionRange::Evaluate()
{
	sptr<Object> target = GetData("target");
	sptr<Object> player = engine->GetPlayer();

	// Player�� ���� ���� ��� ���Ŀ� engine->GetPlayer�� �����Ѵ�.
	if ((mObject->GetPosition() - player->GetPosition()).Length() < mEnemyMgr->mDetectionRange) {
		if (!target) {
			mRoot->SetData("target", player);
		}

		mEnemyMgr->mController->SetValue("Walk", true);
		return BT::NodeState::Success;
	}
	else {
		if (target)
			mRoot->ClearData("target");

		return BT::NodeState::Failure;
	}
}
