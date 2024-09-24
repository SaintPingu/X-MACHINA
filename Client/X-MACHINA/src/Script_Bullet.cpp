#include "stdafx.h"
#include "Script_Bullet.h"

#include "Script_Enemy.h"
#include "Script_Weapon.h"
#include "Script_PlayerController.h"

#include "GameFramework.h"
#include "Object.h"
#include "Timer.h"
#include "BattleScene.h"
#include "SoundMgr.h"

#include "Component/Collider.h"
#include "Component/ParticleSystem.h"
#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"


void Script_Bullet::SetParticleSystems(BulletPSType type, const std::vector<std::string>& psNames)
{
	for (auto& name : psNames) {
		mPSNames[static_cast<UINT8>(type)].emplace_back(name);
	}
}

void Script_Bullet::SetPlayerBullet(bool val, const Object* player)
{
	mIsPlayerBullet = val;
	if (val && player) {
		mPlayerController = player->GetComponent<Script_PlayerController>().get();
	}
	else {
		mPlayerController = nullptr;
	}
}

void Script_Bullet::Update()
{
	base::Update();

	const float distance = mSpeed * DeltaTime();

	mCurDistance += distance;
	mObject->MoveForward(distance);
	
	if (mCurDistance >= mMaxDistance) {
		if (Math::IsEqual(mMaxDistance, mEndDistance)) {
			Disappear();
		}
		else {
			mObject->MoveForward(mMaxDistance - mCurDistance - 0.5f);	// return to impact point
			Explode();
		}
	}
}

void Script_Bullet::Init()
{
	mInstObject = mObject->GetObj<InstObject>();

	Reset();
}

void Script_Bullet::Fire(const Vec3& pos, const Vec3& dir)
{
	mBeforePos = pos;
	mObject->SetPosition(pos);
	mObject->SetLook(dir);

	SetDamage(GetDamage());

	Ray ray{ pos, dir };
	ray.Direction.Normalize();

	static const ObjectTag kCollisionTag = ObjectTag::Building | ObjectTag::DissolveBuilding | ObjectTag::Enemy | ObjectTag::Bound;

	GridObject* target{};
	const std::vector<sptr<Grid>>& grids = BattleScene::I->GetNeighborGrids(BattleScene::I->GetGridIndexFromPos(pos), true);
	for (const auto& grid : grids) {
		for (const auto& other : grid->GetObjects()) {
			ObjectTag otherTag = other->GetTag();
			if (!(otherTag & kCollisionTag)) {
				continue;
			}

			float distance = 0.f;
			if (!other->GetCollider()->Intersects(ray, distance)) {
				continue;
			}
			if (otherTag == ObjectTag::Untagged) {
				continue;
			}
			if (otherTag == ObjectTag::Enemy) {
				const auto& script = other->GetComponent<Script_LiveObject>();
				if (!script || script->IsDead()) {
					continue;
				}
			}

			if (distance < mMaxDistance) {
				mMaxDistance = distance;
				target = other;
			}
			
		}
	}

	if (target) {
		if (target->GetTag() == ObjectTag::Enemy) {
			const auto& enemy = target->GetComponent<Script_Enemy>().get();
			enemy->Hit(0, nullptr);
			mParticleType = BulletPSType::Explosion;
			if (mIsPlayerBullet) {
				if (mPlayerController) {
					mPlayerController->ActiveHitAim();
				}
				// TODO : send onhit packet here
				auto cpkt = FBS_FACTORY->CPkt_Bullet_OnHitEnemy(enemy->GetObj()->GetID(), mObject->GetPosition(), mObject->GetLook());
				CLIENT_NETWORK->Send(cpkt);
			}
		}
		else {
			mParticleType = BulletPSType::Building;
		}
	}
}

void Script_Bullet::Fire(const Transform& transform, const Vec2& err)
{
	mCurDistance = 0.f;
	mObject->SetLocalRotation(transform.GetRotation());
	Vec3 dir = ApplyErr(transform.GetLook(), err);
	Fire(transform.GetPosition(), dir);
	StartFire();

	PlayPSs(BulletPSType::Contrail);
}

void Script_Bullet::Explode()
{
	Disappear();

	if (mImpactSound != "") {
		SoundMgr::I->Play("Effect", mImpactSound);
	}

	PlayPSs(mParticleType);
}

void Script_Bullet::PlayPSs(BulletPSType type)
{
	for (const auto& psName : mPSNames[static_cast<UINT8>(type)]) {
		auto* ps = ParticleManager::I->Play(psName, mObject);

		// 루핑이 설정되어 있다면 직접 해제해야 한다.
		if (ps && ps->GetPSCD()->Looping) {
			mPSs[static_cast<UINT8>(type)].push(ps);
		}
	}
}

void Script_Bullet::StopPSs(BulletPSType type)
{
	while (!mPSs[static_cast<UINT8>(type)].empty()) {
		mPSs[static_cast<UINT8>(type)].front()->Stop();
		mPSs[static_cast<UINT8>(type)].pop();
	}
}

void Script_Bullet::Hit(Script_LiveObject* target)
{
	if (target) {
		target->Hit(GetDamage());
	}
}

Vec3 Script_Bullet::ApplyErr(const Vec3& dir, const Vec2& err)
{
	mObject->Rotate(err.y, err.x, 0.f);
	return Vector3::Rotate(dir, err.y, err.x, 0.f);
}

void Script_Bullet::Disappear()
{
	Reset();
	StopPSs(BulletPSType::Contrail);
	mInstObject->Return();
}

void Script_Bullet::Reset()
{
	mCurDistance = 0.f;
	mMaxDistance = mEndDistance;
}