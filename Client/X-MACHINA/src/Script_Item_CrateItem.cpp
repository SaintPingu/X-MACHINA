#include "stdafx.h"
#include "Script_Item.h"

#include "Script_Player.h"

#include "Component/Rigidbody.h"

#include "Timer.h"
#include "Object.h"



void Script_Item_CrateItem::Awake()
{
	mRigid = mObject->AddComponent<Rigidbody>();
	mMaxFloatingSpeed = 0.25f;
}

void Script_Item_CrateItem::Animate()
{
	if (!mOpened) {
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


void Script_Item_CrateItem::StartOpen()
{
	mOpened = true;
	mRigid->AddForce(Vector3::Up * 1.5f, ForceMode::Impulse);
}

void Script_Item_CrateItem::Interact(Object* user)
{
	auto player = user->GetComponent<Script_GroundPlayer>();
	player->AquireWeapon(mWeaponName);

	mObject->Destroy();
}
