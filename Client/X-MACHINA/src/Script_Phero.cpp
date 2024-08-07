#include "stdafx.h"
#include "Script_Phero.h"

#include "Timer.h"
#include "Object.h"
#include "Component/Rigidbody.h"
#include "BattleScene.h"
#include "GameFramework.h"
#include "XLManager.h"

#include "Script_Player.h"

#include "ClientNetwork/Contents/ClientNetworkManager.h"

void Script_Phero::Start()
{
	GameObject* object = dynamic_cast<GameObject*>(mObject);
	if (nullptr != object) {
		object->SetUseShadow(false);
	}

	mPickupRange = 1.5f; // 추후에 업그레이드 느낌으로 플레이어가 들고 있어도 됨

	mState = PheroState::Spread;
	//mRigid = mObject->AddComponent<Rigidbody>();
	//mRigid->AddForce(Vec3{Math::RandFloat(-1.f, 1.f), 1.5f, Math::RandFloat(-1.f, 1.f)}, Math::RandFloat(8.f, 10.f), ForceMode::Impulse);
	//mRigid->SetGravity(true);
	//mRigid->SetMass(2.f);

	mSpreadStart = mObject->GetPosition();
	mSpreadDest = mSpreadStart + Math::RandVec2(GetID()).xz();
}


void Script_Phero::Update()
{
	mCurrTime += DeltaTime();

	switch (mState)
	{
	case PheroState::Spread:
		if (SpreadAllDirections()) {
			mState = PheroState::Idle;
			//mObject->RemoveComponent<Rigidbody>();
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
		constexpr float kDis = 0.3f;
		if (Vec3::Distance(mObject->GetPosition().xz(), mTarget->GetPosition().xz()) < kDis) {
			auto& pheroPlayer = mTarget->GetComponent<Script_PheroPlayer>();
			if (pheroPlayer) {
				pheroPlayer->AddPheroAmount(mStat.Amount);
			}

			mObject->Destroy();
		}
	}
}

void Script_Phero::SetPheroStat(int level)
{
	XLManager::I->Set(level, mStat);
}

void Script_Phero::SetID(int monster_id, int phero_index)
{
	mObject->SetID((monster_id - 1) * (PheroDropInfo::gkMaxPheroDrop + 1) + phero_index);
}

bool Script_Phero::IntersectTerrain()
{
	if (mRigid->GetVelocity().y > 0.f) {
		return false;
	}

	const Vec3& pos = mObject->GetPosition();
	const float terrainHeight = BattleScene::I->GetTerrainHeight(pos.x, pos.z);

	constexpr float kAdjHeight = 1.f;

	if (pos.y <= terrainHeight + kAdjHeight) {
		mSpreadDest = pos;
		return true;
	}

	return false;
}

bool Script_Phero::SpreadAllDirections()
{
	const float spreadTime = mCurrTime * 2.f;

	const Vec3 center = (mSpreadStart + mSpreadDest) / 2.f;
	const Vec3 dist = mSpreadStart - center;

	const Vec3 cp1 = (mSpreadStart - dist) + Vec3{ 0.f, -10.f, 0.f };
	const Vec3 cp4 = (mSpreadDest + dist) + Vec3{ 0.f, -10.f, 0.f };

	const Vec3 res = Vec3::CatmullRom(cp1, mSpreadStart, mSpreadDest, cp4, spreadTime);
	mObject->SetPosition(res);

	if (spreadTime >= 1.f) {
		return true;
	}

	return false;
}

void Script_Phero::FloatGently()
{
	constexpr float kFloatSpeed = 3.f;
	constexpr float kAdjValue = 2.f;
	constexpr float kFloatRange = 0.1f;

	Vec3 pos = mSpreadDest;
	pos.y = (sinf(mCurrTime * kFloatSpeed) + kAdjValue) * kFloatRange;

	mObject->SetPosition(pos);
}

bool Script_Phero::CheckPlayerRange()
{
	const Vec3& pos = mObject->GetPosition();

	float min = FLT_MAX;
	GridObject* target = nullptr;
	for (const auto& [id, player] : CLIENT_NETWORK->GetRemotePlayers()) {
		float dist = Vec3::Distance(pos, player->GetPosition());
		if (dist < min) {
			min = dist;
			target = player;
		}
	}

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

	constexpr float kAdjTotalSpeed = 5.f;
	constexpr float kAdjFllowSpeed = 4.f;

	const float newFllowSpeed = (powf(mFllowSpeed * kAdjFllowSpeed, 2) - 1.f) * kAdjTotalSpeed;

	const Vec3 dir = mTarget->GetPosition().xz() - mObject->GetPosition().xz();
	mObject->Translate(dir, newFllowSpeed * DeltaTime());
}
