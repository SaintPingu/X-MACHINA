#include "stdafx.h"
#include "Script_Item.h"

#include "Script_Player.h"
#include "Script_Weapon.h"

#include "Component/Rigidbody.h"

#include "Timer.h"
#include "Object.h"

#include "ClientNetwork/Contents/Script_NetworkRemotePlayer.h"


void Script_Item_Weapon::Awake()
{
	base::Awake();

	mRigid = mObject->AddComponent<Rigidbody>();
	mMaxFloatingSpeed = 0.25f;
	mObject->mObjectCB.UseOutline = true;
}

void Script_Item_Weapon::Start()
{
	mObject->DetachParent(false);
	mObject->SetLocalRotation(Vec3(0, 90, 0));
	mObject->SetTag(ObjectTag::Item);

	StartDrop();
}

void Script_Item_Weapon::Animate()
{
	base::Animate();

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
	//mRigid->AddForce(Vector3::Up * 1.5f, ForceMode::Impulse);
}

void Script_Item_Weapon::StartDrop()
{
	mObject->SetPositionY(0.5f);
}

bool Script_Item_Weapon::Interact()
{
	if (mDroped) {
		return false;
	}

	return true;
}

void Script_Item_Weapon::InteractOK(Object* user)
{
	const auto& weapon = mObject->GetComponent<Script_Weapon>();

	if (weapon) {
		mObject->SetTag(ObjectTag::Untagged);
		mObject->SetActive(false);
		mObject->mObjectCB.UseOutline = false;
		mObject->RemoveComponent<Script_Item_Weapon>();
	}
	else {
		std::cout << "[WARNING] (weapon item or player) has no script\n";
		mObject->Destroy();
	}


	const auto& player = user->GetComponent<Script_GroundPlayer>();
	if (player) {
		if (player) {
			player->TakeWeapon(weapon);
			
		}
		else {
			const auto& removeplayer = user->GetComponent<Script_NetworkRemotePlayer>();
			if (removeplayer) {
				removeplayer->TakeWeapon(weapon);
			}
		}
	}
}
