#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

class GameObject;
class Rigidbody;
class UI;

enum class ItemType {
	None,
	WeaponCrate,
	Weapon
};



class Script_Item abstract : public Component {
	COMPONENT_ABSTRACT(Script_Item, Component)

private:
	sptr<UI> mUI{};
	bool mCanInteract{};

public:
	virtual void Awake() override;
	virtual void Update() override;
	virtual void OnCollisionEnter(Object& other) override;
	virtual void OnCollisionExit(Object& other) override;

public:
	virtual bool Interact(Object* user) abstract;
	virtual ItemType GetItemType() abstract;

protected:
	virtual void DisableInteract();
};



class Script_Item_WeaponCrate : public Script_Item, SceneScript {
	COMPONENT(Script_Item_WeaponCrate, Script_Item)

public:
	WeaponName mWeaponName;
	Transform* mCap{};

	float mCapPitch{};
	bool mIsOpend{};

	GameObject* mWeapon{};

public:
	virtual void Awake() override;
	virtual void Animate() override;
	virtual void OnCollisionEnter(Object& other) override;

public:
	virtual bool Interact(Object* user) override;
	virtual ItemType GetItemType() override { return ItemType::WeaponCrate; }

	virtual void LoadData(rsptr<ScriptExporter> exporter) override;

protected:
	virtual void DisableInteract();
};



class Script_Item_Weapon : public Script_Item {
	COMPONENT_ABSTRACT(Script_Item_Weapon, Script_Item)

private:
	sptr<Rigidbody> mRigid{};
	Vec3 mDir{};

	bool mDroped{ false };

	int mSign{ 1 };
	float mDeltaTime{};
	float mMaxFloatingSpeed{};

public:
	virtual void Awake() override;
	virtual void Animate() override;

public:
	virtual bool Interact(Object* user) override;
	virtual ItemType GetItemType() override { return ItemType::Weapon; }

public:
	virtual void StartOpen();
	virtual void StartDrop();
};