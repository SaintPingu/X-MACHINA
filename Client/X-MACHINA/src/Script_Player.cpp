#include "stdafx.h"
#include "Script_Player.h"

#include "Script_MainCamera.h"
#include "Script_Item.h"

#include "Component/Camera.h"
#include "Component/Rigidbody.h"
#include "Component/UI.h"

#include "Object.h"

#include "SliderBarUI.h"
#include "ChatBoxUI.h"


#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"

void Script_Player::Start()
{
	base::Start();

	mCamera = MainCamera::I->GetComponent<Script_MainCamera>().get();
}

void Script_Player::Update()
{
	base::Update();
}

void Script_Player::SetSpawn(const Vec3& pos)
{
	mObject->SetPosition(pos);
	XMStoreFloat4x4(&mSpawnTransform, _MATRIX(mObject->GetWorldTransform()));
}

bool Script_Player::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID) {
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'E':
			Interact();
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return true;
}

void Script_Player::Rotate(float pitch, float yaw, float roll)
{
	mObject->Rotate(pitch, yaw, roll);
}

void Script_Player::Dead()
{
	Respawn();
}

void Script_Player::Respawn()
{
	Resurrect();
	mObject->SetWorldTransform(mSpawnTransform);
}

void Script_Player::Interact()
{
	Script_Item* interactedItem{};
	const auto& collisionObjects = mObject->GetCollisionObjects();
	for (const auto& other : collisionObjects) {
		switch (other->GetTag()) {
		case ObjectTag::Crate:
		case ObjectTag::Item:
			if (!interactedItem) {
				interactedItem = other->GetComponent<Script_Item>().get();
				if (interactedItem->Interact()) {
					goto label_end;
				}
				interactedItem = nullptr;
			}
			break;
		default:
			break;
		}
	}
label_end:
	if (interactedItem) {
		auto itemType = FBS_FACTORY->GetItemType(interactedItem->GetItemType());
		auto cpkt = FBS_FACTORY->CPkt_Item_Interact(interactedItem->GetObj()->GetID(), itemType);
		CLIENT_NETWORK->Send(cpkt);
	}
}
