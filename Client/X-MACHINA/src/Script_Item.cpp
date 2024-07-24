#include "stdafx.h"
#include "Script_Item.h"

#include "Script_Player.h"

#include "Component/UI.h"
#include "Component/Camera.h"

void Script_Item::Awake()
{
	base::Awake();

	mUI = Canvas::I->CreateUI<UI>(3, "Key_E", Vec2::Zero, Vec2(20, 20));
	mUI->SetActive(false);
}

void Script_Item::Update()
{
	base::Update();

	if (mCanInteract) {
		Vec2 pos = MAIN_CAMERA->WorldToScreenPoint(mObject->GetPosition());
		pos.y += 50.f;
		mUI->SetPosition(pos);
	}
}

void Script_Item::OnDestroy()
{
	base::OnDestroy();

	Canvas::I->RemoveUI(3, mUI);
}

void Script_Item::OnCollisionEnter(Object& other)
{
	base::OnCollisionEnter(other);

	if (other.GetTag() == ObjectTag::Player) {
		if (other.GetComponent<Script_Player>()) {
			mCanInteract = true;
			mUI->SetActive(true);
		}
	}
}

void Script_Item::OnCollisionExit(Object& other)
{
	base::OnCollisionExit(other);

	if (other.GetTag() == ObjectTag::Player) {
		if (other.GetComponent<Script_Player>()) {
			DisableInteract();
		}
	}
}

void Script_Item::DisableInteract()
{
	mCanInteract = false;
	mUI->SetActive(false);
}
