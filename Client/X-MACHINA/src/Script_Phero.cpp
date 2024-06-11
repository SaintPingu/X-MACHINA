
#include "stdafx.h"
#include "Script_Phero.h"

#include "Timer.h"
#include "Object.h"
#include "Component/Rigidbody.h"
#include "Scene.h"
#include "GameFramework.h"
#include "XLManager.h"


void Script_Phero::Start()
{
	GameObject* object = dynamic_cast<GameObject*>(mObject);
	if (nullptr != object) {
		object->SetUseShadow(false);
	}

	mPickupRange = 1.5f;	// 추후에 업그레이드 느낌으로 플레이어가 들고 있어도 됨

	mState = PheroState::Spread;
	mRigid = mObject->AddComponent<Rigidbody>();
	mRigid->AddForce(Vec3{Math::RandFloat(-1.f, 1.f), 1.5f, Math::RandFloat(-1.f, 1.f)}, Math::RandFloat(8.f, 10.f), ForceMode::Impulse);
	mRigid->SetGravity(true);
	mRigid->SetMass(2.f);
}


void Script_Phero::Update()
{
	mCurrTime += DeltaTime();

	switch (mState)
	{
	case PheroState::Spread:
		if (IntersectTerrain()) {
			mState = PheroState::Idle;
			mObject->RemoveComponent<Rigidbody>();
		}
		break;
	case PheroState::Idle:
		FloatGently();
		if (CheckPlayerRange()) {
			mState = PheroState::Follow;
		}
		break;
	case PheroState::Follow:
		FollowToTarget();
		break;
	default:
		break;
	}
}

void Script_Phero::LateUpdate()
{
	if (mState != PheroState::Follow && mCurrTime >= mStat.LifeTime) {
		mObject->Destroy();
	}

	if (mState == PheroState::Follow) {
		const float dis = 0.3f;
		if (Vec3::Distance(mObject->GetPosition().xz(), mTarget->GetPosition().xz()) < dis) {
			// TODO : 타겟의 페로 증가
			mObject->Destroy();
		}
	}
}

void Script_Phero::SetPheroStat(int level)
{
	XLManger::I->Set(level, mStat);
}

bool Script_Phero::IntersectTerrain()
{
	if (mRigid->GetVelocity().y > 0.f) {
		return false;
	}

	const Vec3& pos = mObject->GetPosition();
	const float terrainHeight = Scene::I->GetTerrainHeight(pos.x, pos.z);

	const float adjHeight = 0.5f;

	if (pos.y <= terrainHeight + adjHeight) {
		mSpreadDest = pos;
		return true;
	}

	return false;
}

void Script_Phero::FloatGently()
{
	const float floatSpeed = 3.f;

	Vec3 pos = mSpreadDest;
	pos.y = (sinf(mCurrTime * floatSpeed) + 1.f) * 0.1f;

	mObject->SetPosition(pos);
}

bool Script_Phero::CheckPlayerRange()
{
	sptr<GridObject> target = GameFramework::I->GetPlayer();
	if (!target) {
		return false;
	}

	const Vec3& targetPos = target->GetPosition();
	if (Vec3::Distance(mObject->GetPosition(), targetPos) > mPickupRange) {
		return false;
	}

	mTarget = target;

	return true;
}

void Script_Phero::FollowToTarget()
{
	if (!mTarget) {
		return;
	}

	mFllowSpeed += DeltaTime();

	const float newFllowSpeed = ((mFllowSpeed * 4.f) * (mFllowSpeed * 4.f) - 1.f) * 5.f;

	const Vec3 dir = mTarget->GetPosition().xz() - mObject->GetPosition().xz();
	mObject->Translate(dir, newFllowSpeed * DeltaTime());
}
