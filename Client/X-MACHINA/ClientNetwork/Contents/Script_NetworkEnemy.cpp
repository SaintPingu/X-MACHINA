#include "stdafx.h"
#include "Script_NetworkEnemy.h"
#include "Timer.h"
#include "Script_EnemyManager.h"
#include "GameFramework.h"
#include "Object.h"
#include "ClientNetworkManager.h"
#include "AnimatorController.h"

void Script_NetworkEnemy::Awake()
{
	base::Awake();

	mEnemyMgr = mObject->GetComponent<Script_EnemyManager>().get();
}

void Script_NetworkEnemy::Update()
{
	base::Update();

	//if (mEnemyMgr->mState == EnemyState::MoveToTarget)
	//	//MoveToTarget();
	//if (mEnemyMgr->mState == EnemyState::Attack)
	//	Attack();
}

void Script_NetworkEnemy::SetPostion(const Vec3& pos)
{
	mObject->SetPosition(pos);
}

void Script_NetworkEnemy::SetRotation(float angle)
{
	mObject->SetLocalRotation(Quat::Identity);
	mObject->Rotate(Vector3::Up, angle);
}

void Script_NetworkEnemy::SetState(EnemyState state)
{
	mEnemyMgr->mState = state;

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
		std::cout << "MoveToTarget" << std::endl;
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

void Script_NetworkEnemy::SetTarget(Object* target)
{
	if (target) {
		mEnemyMgr->mTarget = target;
	}
}

EnemyState Script_NetworkEnemy::GetState()
{
	return mEnemyMgr->mState;
}

void Script_NetworkEnemy::MoveToTarget()
{
	if (!mEnemyMgr->mTarget) {
		return;
	}

	// �㸮 �ʺ��� ������ ���� �´´�.
	Vec3 objectAdjPos = mObject->GetPosition() + mObject->GetUp() * 0.5f;
	Vec3 targetAdjPos = mEnemyMgr->mTarget->GetPosition() + mEnemyMgr->mTarget->GetUp() * 0.5f;

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	Vec3 toTarget = targetAdjPos - objectAdjPos;

	if (toTarget.Length() < mEnemyMgr->mStat.AttackRange) {
		return;
	}

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	const float kMinDistance = 0.1f;

	// Ÿ�ٿ� �������� �ʾ��� ��쿡�� �̵�
	if (toTarget.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(mEnemyMgr->mTarget->GetPosition(), mEnemyMgr->mStat.RotationSpeed);
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mStat.MoveSpeed * DeltaTime());
	}

	mEnemyMgr->mController->SetValue("Attack", false);
	mEnemyMgr->mController->SetValue("Walk", true);
}

void Script_NetworkEnemy::Attack()
{
	if (!mEnemyMgr->mTarget) {
		return;
	}

	mObject->RotateTargetAxisY(mEnemyMgr->mTarget->GetPosition(), mEnemyMgr->mStat.AttackRotationSpeed);
	mEnemyMgr->mController->SetValue("Walk", false);
	mEnemyMgr->mController->SetValue("Attack", true);
}
