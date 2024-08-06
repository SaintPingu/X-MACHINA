#include "stdafx.h"
#include "Script_SpiderMine.h"

#include "Script_LiveObject.h"
#include "Script_MainCamera.h"

#include "Component/Collider.h"
#include "Component/Camera.h"

#include "Object.h"
#include "BattleScene.h"
#include "Timer.h"


void Script_SpiderMine::Awake()
{
	base::Awake();
	mMass = 0.3f;
	mDrag = 3.f;
	mRotationSpeed = 90.f;

	SetExplosionTag(ObjectTag::Enemy);
}

void Script_SpiderMine::Update()
{
	if (mIsPlanted) {
		mObject->Rotate(0, mRotationSpeed * DeltaTime(), 0);
		return;
	}

	base::Update();

	mObject->MoveUp(-Math::kGravity * mMass * DeltaTime());
	if (mObject->GetPosition().y <= 0.2f) {
		Plant();
	}

	mSpeed -= Math::Sign(mSpeed) * DeltaTime() * mDrag;
}

void Script_SpiderMine::OnCollisionEnter(Object& other)
{
	if (!mIsPlanted) {
		switch (other.GetTag()) {
		case ObjectTag::Bound:
		case ObjectTag::Building:
		case ObjectTag::DissolveBuilding:
			mSpeed *= -1;
			break;
		}
		return;
	}

	base::OnCollisionEnter(other);
}

void Script_SpiderMine::Fire(const Vec3& pos, const Vec3& dir)
{
	mObject->SetPosition(pos);
	mObject->SetLook(dir);
	mObject->Rotate(-20, 0, 0);

	SetDamage(GetDamage());
}

void Script_SpiderMine::StartFire()
{
	base::StartFire();

	for (const auto& light : mLights) {
		light->SetActive(false);
	}
}

void Script_SpiderMine::Plant()
{
	mIsPlanted = true;
	mObject->Rotate(20, 0, 0);
	mObject->SetPositionY(0.2f);
}
