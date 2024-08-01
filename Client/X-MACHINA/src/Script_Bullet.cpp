#include "stdafx.h"
#include "Script_Bullet.h"

#include "Script_Enemy.h"
#include "Script_Weapon.h"

#include "Object.h"
#include "Timer.h"
#include "BattleScene.h"
#include "SoundMgr.h"

#include "Component/Collider.h"
#include "Component/ParticleSystem.h"

void Script_Bullet::SetParticleSystems(BulletPSType type, const std::vector<std::string>& psNames)
{
	for (auto& name : psNames) {
		mPSNames[static_cast<UINT8>(type)].emplace_back(name);
	}
}

void Script_Bullet::Update()
{
	base::Update();

	const float distance = mSpeed * DeltaTime();

	mCurDistance += distance;
	mObject->MoveForward(distance);

	if (mCurDistance >= mMaxDistance) {
		if (Math::IsEqual(mMaxDistance, mkMaxDistance)) {
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
	mObject->SetTag(ObjectTag::Bullet);

	Reset();
}

void Script_Bullet::Fire(const Vec3& pos, const Vec3& dir, const Vec3& up)
{
	mObject->SetPosition(pos);
	mObject->SetLook(dir);

	SetDamage(GetDamage());

	Ray ray{ pos, dir };
	ray.Direction.Normalize();

	static const ObjectTag kCollisionTag = ObjectTag::Building | ObjectTag::DissolveBuilding | ObjectTag::Enemy | ObjectTag::Bound | ObjectTag::Player;

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

			if (distance < mMaxDistance) {
				mMaxDistance = distance;
				target = other;
			}
		}
	}

	if (target) {
		if (target->GetTag() == ObjectTag::Enemy) {
			const auto& enemy = target->GetComponent<Script_Enemy>().get();
			Hit(enemy);
			mParticleType = BulletPSType::Explosion;
		}
		else {
			mParticleType = BulletPSType::Building;
		}
	}
}

void Script_Bullet::Fire(const Transform& transform, const Vec2& err)
{
	PlayPSs(BulletPSType::Contrail);

	mObject->SetLocalRotation(transform.GetRotation());
	Vec3 dir = transform.GetLook();
	dir = Vector3::Rotate(dir, err.y, err.x, 0.f);
	mObject->Rotate(err.y, err.x, 0.f);
	Fire(transform.GetPosition(), dir, transform.GetUp());
	StartFire();
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

void Script_Bullet::Disappear()
{
	Reset();
	StopPSs(BulletPSType::Contrail);
	mInstObject->Return();
}

void Script_Bullet::Reset()
{
	mCurDistance = 0.f;
	mMaxDistance = mkMaxDistance;
}