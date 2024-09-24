#include "stdafx.h"
#include "Script_Missile.h"

#include "Object.h"
#include "Component/Collider.h"
#include "Component/ParticleSystem.h"


void Script_Missile::Awake()
{
	base::Awake();

	SetExplosionTag(ObjectTag::Enemy | ObjectTag::Bound | ObjectTag::Building | ObjectTag::DissolveBuilding);
}

void Script_Missile::Update()
{
	if (mObject->GetPosition().y <= 0) {
		Explode();
		return;
	}

	base::Update();
}

void Script_Missile::Fire(const Vec3& pos, const Vec3& dir)
{
	mObject->SetPosition(pos);
	mObject->SetLook(dir);
	mObject->GetComponent<ObjectCollider>()->UpdateTransform();

	SetDamage(GetDamage());
}

void Script_Missile::Fire(const Vec3& pos, const Vec3& dir, const Vec2& err)
{
	Fire(pos, dir);
	ApplyErr(dir, err);

	mContrail = ParticleManager::I->Play("WFX_Bullet", mObject);
}

void Script_Missile::Explode()
{
	base::Explode();

	if (mContrail) {
		mContrail->Stop();
		mContrail = nullptr;
	}
}

void Script_DeusMissile::Awake()
{
	base::Awake();

	SetExplosionTag(ObjectTag::Player | ObjectTag::Bound | ObjectTag::Building | ObjectTag::DissolveBuilding);
}
