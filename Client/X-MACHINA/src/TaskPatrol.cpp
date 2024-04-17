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
	mPatrolSpeed = mEnemyMgr->mMoveSpeed * 0.5f;

	mWayPoints = wayPoints;
}


BT::NodeState TaskPatrol::Evaluate()
{
	Vec3 wayPoint = mWayPoints[mCurrWayPointIdx];

	const float kMinDistance = 1.f;
	const Vec3 toWayPoint = wayPoint - mObject->GetPosition().xz();
	
	if (toWayPoint.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(wayPoint, mEnemyMgr->mRotationSpeed);
		mObject->Translate(mObject->GetLook(), mPatrolSpeed * DeltaTime());
	}
	else {
		mCurrWayPointIdx = (mCurrWayPointIdx + 1) % mWayPoints.size();
	}

	return BT::NodeState::Running;
}
