#include "stdafx.h"
#include "Script_Missile.h"

#include "Script_LiveObject.h"
#include "Script_MainCamera.h"

#include "Component/Collider.h"
#include "Component/Camera.h"

#include "Object.h"
#include "BattleScene.h"


void Script_Missile::Update()
{
	if (mObject->GetPosition().y <= 0) {
		Explode();
		return;
	}

	base::Update();
}

void Script_Missile::OnCollisionEnter(Object& other)
{
	if (IsOwner(&other)) {
		return;
	}

	switch (other.GetTag()) {
	case ObjectTag::Enemy:
	case ObjectTag::Bound:
	case ObjectTag::Building:
	case ObjectTag::DissolveBuilding:
		Explode();
		break;

	default:
		break;
	}
}

void Script_Missile::Fire(const Vec3& pos, const Vec3& dir)
{
	mObject->SetPosition(pos);
	mObject->SetLook(dir);

	SetDamage(GetDamage());
}

void Script_Missile::Fire(const Vec3& pos, const Vec3& dir, const Vec2& err)
{
	Fire(pos, dir);
	ApplyErr(dir, err);
}

void Script_Missile::Init()
{
	base::Init();

	mExplosionCollider = mObject->FindFrame("Explosion_Collider")->GetObj<Object>()->GetComponent<SphereCollider>();
}

void Script_Missile::Explode()
{
	base::Explode();

	if (mExplosionCollider) {
		mExplosionCollider->SetActive(true);
	}

	std::vector<GridObject*> collisionObjects{};
	BattleScene::I->CheckCollisionCollider(mExplosionCollider, collisionObjects, CollisionType::Dynamic);
	for (const auto& object : collisionObjects) {
		if (object == mObject) {
			continue;
		}

		if (object->GetTag() == ObjectTag::Enemy || object->GetTag() == ObjectTag::Player) {
			//object->GetComponent<Script_LiveObject>()->Hit(mExplosionDamage);
		}
	}

	MainCamera::I->GetComponent<Script_MainCamera>()->StartShake(1.f, 0.001f);
}

void Script_Missile::StartFire()
{
	base::StartFire();

	if (mExplosionCollider) {
		mExplosionCollider->SetActive(false);
	}
}
