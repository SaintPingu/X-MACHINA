#include "stdafx.h"
#include "Script_Missile.h"

#include "GameFramework.h"

#include "Script_LiveObject.h"
#include "Script_GameManager.h"
#include "Script_MainCamera.h"

#include "Component/Collider.h"

#include "Object.h"
#include "Scene.h"


void Script_Missile::Init()
{
	base::Init();

	mExplosionCollider = mObject->FindFrame("Explosion_Collider")->GetObj<Object>()->GetComponent<SphereCollider>();
}

void Script_Missile::Explode()
{
	if (mExplosionCollider) {
		mExplosionCollider->SetActive(true);
	}

	std::vector<GridObject*> collisionObjects{};
	Scene::I->CheckCollisionCollider(mExplosionCollider, collisionObjects, CollisionType::Dynamic);
	for (const auto& object : collisionObjects) {
		if (object == mObject) {
			continue;
		}

		if (object->GetTag() == ObjectTag::Enemy || object->GetTag() == ObjectTag::Player) {
			object->GetComponent<Script_LiveObject>()->Hit(mExplosionDamage);
		}
	}

	GameFramework::I->GetGameManager()->GetCamera()->StartShake(1.f, 0.001f);

	base::Explode();

}

void Script_Missile::StartFire()
{
	base::StartFire();

	if (mExplosionCollider) {
		mExplosionCollider->SetActive(false);
	}
}
