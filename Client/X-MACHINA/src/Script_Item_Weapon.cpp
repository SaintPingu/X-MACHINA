#include "stdafx.h"
#include "Script_Item.h"

#include "Script_Player.h"
#include "Script_Weapon.h"

#include "Component/Rigidbody.h"

#include "Timer.h"
#include "Object.h"



void Script_Item_Weapon::Awake()
{
	mRigid = mObject->AddComponent<Rigidbody>();
	mMaxFloatingSpeed = 0.25f;
	mObject->mObjectCB.UseOutline = true;
}

void Script_Item_Weapon::Animate()
{
	if (!mDroped) {
		return;
	}

	float floatingSpeed = mMaxFloatingSpeed * sin(mDeltaTime * XM_PI);

	mObject->SetPositionY(mObject->GetPosition().y + (floatingSpeed * mSign * DeltaTime()));

	mDeltaTime += DeltaTime();
	if (mDeltaTime >= 1.f) {
		mDeltaTime = 0.f;
		mSign *= -1;
	}
}


void Script_Item_Weapon::StartOpen()
{
	mDroped = true;
	mRigid->AddForce(Vector3::Up * 1.5f, ForceMode::Impulse);
}

void Script_Item_Weapon::StartDrop()
{
	mObject->SetPositionY(0.5f);
	mDroped = true;
}

bool Script_Item_Weapon::Interact(Object* user)
{
	const auto& player = user->GetComponent<Script_GroundPlayer>();
	const auto& weapon = mObject->GetComponent<Script_Weapon>();

	if (weapon) {
		player->TakeWeapon(weapon);
		mObject->RemoveComponent<Script_Item_Weapon>();
		mObject->SetTag(ObjectTag::Unspecified);
		mObject->SetActive(false);
		mObject->mObjectCB.UseOutline = false;
	}
	else {
		std::cout << "[ERROR] weapon item has no script\n";
		mObject->Destroy();
	}

	return true;
}
