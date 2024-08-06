#include "stdafx.h"
#include "Script_ExplosiveBullet.h"

#include "Script_LiveObject.h"
#include "Script_MainCamera.h"

#include "Component/Collider.h"
#include "Component/Camera.h"

#include "Object.h"
#include "BattleScene.h"


void Script_ExplosiveBullet::OnCollisionEnter(Object& other)
{
	base::OnCollisionEnter(other);

	if (other.GetTag() & mExplosionTag) {
		Explode();
	}
}

void Script_ExplosiveBullet::Init()
{
	base::Init();

	mExplosionCollider = mObject->FindFrame("Explosion_Collider")->GetObj<Object>()->GetComponent<SphereCollider>();
}

void Script_ExplosiveBullet::Explode()
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

void Script_ExplosiveBullet::StartFire()
{
	base::StartFire();

	if (mExplosionCollider) {
		mExplosionCollider->SetActive(false);
	}
}