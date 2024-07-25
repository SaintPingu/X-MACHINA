#include "stdafx.h"
#include "Script_EnemyNetwork.h"
#include "Timer.h"
#include "Script_EnemyManager.h"
#include "GameFramework.h"
#include "Object.h"
#include "ClientNetworkManager.h"

void Script_EnemyNetwork::Awake()
{
	mEnemyMgr = mObject->GetComponent<Script_EnemyManager>().get();
}

void Script_EnemyNetwork::Update()
{
	switch (mEnemyMgr->mState)
	{
	case EnemyState::Idle:
		std::cout << "Idle" << std::endl;
		break;
	case EnemyState::Walk:
		std::cout << "Walk" << std::endl;
		break;
	case EnemyState::GetHit:
		std::cout << "GetHit" << std::endl;
		break;
	case EnemyState::Attack:
		std::cout << "Attack" << std::endl;
		break;
	case EnemyState::Death:
		std::cout << "Death" << std::endl;
		break;
	case EnemyState::MoveToTarget:
		MoveToTarget();
		break;
	case EnemyState::MoveToPath:
		std::cout << "MoveToPath" << std::endl;
		break;
	case EnemyState::Patrol:
		std::cout << "Patrol" << std::endl;
		break;
	default:
		break;
	}
}

void Script_EnemyNetwork::SetPostion(const Vec3& pos)
{
	mObject->SetPosition(pos);
}

void Script_EnemyNetwork::SetRotation(const Vec3& rot)
{
	mObject->SetLocalRotation(Quaternion::ToQuaternion(rot));
}

void Script_EnemyNetwork::SetState(EnemyState state)
{
	mEnemyMgr->mState = state;
}

void Script_EnemyNetwork::SetTarget(Object* target)
{
	if (target) {
		mEnemyMgr->mTarget = target;
	}
}

void Script_EnemyNetwork::MoveToTarget()
{
	if (!mEnemyMgr->mTarget) {
		return;
	}

	// 허리 쪽부터 광선을 쏴야 맞는다.
	Vec3 objectAdjPos = mObject->GetPosition() + mObject->GetUp() * 0.5f;
	Vec3 targetAdjPos = mEnemyMgr->mTarget->GetPosition() + mEnemyMgr->mTarget->GetUp() * 0.5f;

	// 오브젝트로부터 타겟까지의 벡터
	Vec3 toTarget = targetAdjPos - objectAdjPos;

	// 오브젝트로부터 타겟까지의 벡터
	const float kMinDistance = 0.1f;

	// 타겟에 도착하지 않았을 경우에만 이동
	if (toTarget.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(mEnemyMgr->mTarget->GetPosition(), mEnemyMgr->mStat.RotationSpeed);
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mStat.MoveSpeed * DeltaTime());
	}
}
