#include "stdafx.h"
#include "TaskPatrol.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


TaskPatrol::TaskPatrol(Object* object, std::vector<Vec3>&& wayPoints)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();

	mWayPoints = wayPoints;
}


BT::NodeState TaskPatrol::Evaluate()
{
	Vec3 wayPoint = mWayPoints[mCurrWayPointIdx];

	mEnemyMgr->mIsMoveToPath = false;

	const float kMinDistance = 1.f;
	const Vec3 toWayPoint = wayPoint - mObject->GetPosition().xz();
	
	if (toWayPoint.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(wayPoint, mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(XMVector3Normalize(toWayPoint), mEnemyMgr->mMoveSpeed * DeltaTime());
	}
	else {
		mEnemyMgr->mController->SetValue("Walk", false);
		mCurrWayPointIdx = (mCurrWayPointIdx + 1) % mWayPoints.size();
	}

	return BT::NodeState::Running;
}
