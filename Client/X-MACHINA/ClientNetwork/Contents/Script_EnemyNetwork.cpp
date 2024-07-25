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

	// �㸮 �ʺ��� ������ ���� �´´�.
	Vec3 objectAdjPos = mObject->GetPosition() + mObject->GetUp() * 0.5f;
	Vec3 targetAdjPos = mEnemyMgr->mTarget->GetPosition() + mEnemyMgr->mTarget->GetUp() * 0.5f;

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	Vec3 toTarget = targetAdjPos - objectAdjPos;

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	const float kMinDistance = 0.1f;

	// Ÿ�ٿ� �������� �ʾ��� ��쿡�� �̵�
	if (toTarget.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(mEnemyMgr->mTarget->GetPosition(), mEnemyMgr->mStat.RotationSpeed);
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mStat.MoveSpeed * DeltaTime());
	}
}
