#include "stdafx.h"
#include "Script_NetworkEnemy.h"
#include "Timer.h"
#include "Script_EnemyManager.h"
#include "Script_Enemy.h"
#include "GameFramework.h"
#include "Object.h"
#include "ClientNetworkManager.h"
#include "AnimatorController.h"
#include "Script_LiveObject.h"

EnemyState Script_NetworkEnemy::GetState()
{
	return mEnemyMgr->mState;
}

void Script_NetworkEnemy::SetPosition(const Vec3& pos)
{
	mObject->SetPosition(pos);
}

void Script_NetworkEnemy::SetRotation(float angle)
{
	mObject->SetLocalRotation(Quat::Identity);
	mObject->Rotate(Vector3::Up, angle);
}

void Script_NetworkEnemy::SetLocalRotation(const Quat& quat)
{
	mObject->SetLocalRotation(quat);
}

void Script_NetworkEnemy::SetState(EnemyState state, int attackCnt)
{
	//if (state == EnemyState::Attack && mEnemyMgr->mState == EnemyState::Attack) {
	//	if (mEnemyMgr->mEnemy->GetCurrAttackCnt() == attackCnt) {
	//		mEnemyMgr->mEnemy->IgnoreAttackEndCallback();
	//	}
	//}
	mEnemyMgr->mState = state;
	SetCurrAttackCnt(attackCnt);
	//std::cout << attackCnt << std::endl;

	//switch (mEnemyMgr->mState)
	//{
	//case EnemyState::Idle:
	//	std::cout << "Idle" << std::endl;
	//	break;
	//case EnemyState::Walk:
	//	std::cout << "Walk" << std::endl;
	//	break;
	//case EnemyState::GetHit:
	//	std::cout << "GetHit" << std::endl;
	//	break;
	//case EnemyState::Attack:
	//	std::cout << "Attack" << std::endl;
	//	break;
	//case EnemyState::Death:
	//	std::cout << "Death" << std::endl;
	//	break;
	//case EnemyState::MoveToTarget:
	//	std::cout << "MoveToTarget" << std::endl;
	//	break;
	//case EnemyState::MoveToPath:
	//	std::cout << "MoveToPath" << std::endl;
	//	break;
	//case EnemyState::Patrol:
	//	std::cout << "Patrol" << std::endl;
	//	break;
	//default:
	//	break;
	//}
}

void Script_NetworkEnemy::SetState(FBProtocol::MONSTER_BT_TYPE btType, int attackCnt)
{
	switch (btType)
	{
	case FBProtocol::MONSTER_BT_TYPE_DEATH:
		SetState(EnemyState::Death, attackCnt);
		break;
	case FBProtocol::MONSTER_BT_TYPE_ATTACK:
		SetState(EnemyState::Attack, attackCnt);
		mEnemy->StartAttack();
		break;
	case FBProtocol::MONSTER_BT_TYPE_GETHIT:
		SetState(EnemyState::GetHit, attackCnt);
		break;
	case FBProtocol::MONSTER_BT_TYPE_MOVE_TO_TARGET:
		mEnemyMgr->mController->SetValue("Walk", true);
		SetState(EnemyState::MoveToTarget, attackCnt);
		break;
	case FBProtocol::MONSTER_BT_TYPE_MOVE_TO_PATH:
		mEnemyMgr->mController->SetValue("Walk", true);
		SetState(EnemyState::MoveToPath, attackCnt);
		break;
	case FBProtocol::MONSTER_BT_TYPE_PATROL:
		mEnemyMgr->mController->SetValue("Walk", true);
		SetState(EnemyState::Patrol, attackCnt);
		break;
	default:
		SetState(EnemyState::Idle, attackCnt);
		break;
	}
}

void Script_NetworkEnemy::SetTarget(Object* target)
{
	mEnemyMgr->mTarget = target;
}

void Script_NetworkEnemy::SetCurrAttackCnt(int attackCnt)
{
	mEnemy->SetCurrAttackCnt(attackCnt);
}

void Script_NetworkEnemy::SetActiveMyObject(bool isActive)
{
	mObject->SetActive(isActive);
}

void Script_NetworkEnemy::Awake()
{
	base::Awake();

	mEnemyMgr = mObject->GetComponent<Script_EnemyManager>().get();
	mEnemy = mObject->GetComponent<Script_Enemy>().get();

	mDeathRemoveTime = 4.f;
}

void Script_NetworkEnemy::Update()
{
	base::Update();

	if (mEnemyMgr->mState == EnemyState::MoveToTarget)
		MoveToTarget();
	else if (mEnemyMgr->mState == EnemyState::Attack)
		Attack();
	else if (mEnemyMgr->mState == EnemyState::Death)
		Death();
	else if (mEnemyMgr->mState == EnemyState::Idle)
		Idle();
}

void Script_NetworkEnemy::MoveToTarget()
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

void Script_NetworkEnemy::Attack()
{
	if (!mEnemyMgr->mTarget) {
		return;
	}

	mEnemy->Attack();
}

void Script_NetworkEnemy::Idle()
{
	mEnemyMgr->RemoveAllAnimation();
}

void Script_NetworkEnemy::Death()
{
	if (!mIsDeath) {
		mIsDeath = true;
		if (auto& script = mObject->GetComponent<Script_LiveObject>()) {
			script->Dead();
		}

		CLIENT_NETWORK->EraseMonster(mObject->GetID());

		mEnemyMgr->RemoveAllAnimation();
		mEnemyMgr->mController->SetValue("Death", true);
	}

	mDeathAccTime += DeltaTime();
	if (mDeathAccTime >= mDeathRemoveTime) {
		mObject->mObjectCB.HitRimFactor = 0.7f;
		mObject->Destroy();
	}
}
