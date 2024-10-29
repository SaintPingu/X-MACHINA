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
	UI* mUI{};
	bool mCanInteract{};

protected:
	int mItemID{};

public:
	virtual void Awake() override;
	virtual void Update() override;
	virtual void OnDestroy() override;
	virtual void OnCollisionEnter(Object& other) override;
	virtual void OnCollisionExit(Object& other) override;

public:
	virtual bool Interact() { return true; }
	virtual bool InteractOK(Object* user) abstract;
	virtual ItemType GetItemType() abstract;

protected:
	virtual void DisableInteract();
};



class Script_Item_WeaponCrate : public Script_Item, SceneScript {
	COMPONENT(Script_Item_WeaponCrate, Script_Item)

private:
	std::string mWeaponName{};

public:
	Transform* mCap{};

	float mCapPitch{};
	bool mIsOpend{};

public:
	virtual void Awake() override;
	virtual void Animate() override;
	virtual void OnCollisionEnter(Object& other) override;

public:
	virtual bool Interact() override;
	virtual bool InteractOK(Object* user) override { mIsOpend = true; return true; }
	virtual ItemType GetItemType() override { return ItemType::WeaponCrate; }

	virtual void LoadData(rsptr<ScriptExporter> exporter) override;

	uint8_t GetWeaponType() const;

protected:
	virtual void DisableInteract();
};



class Script_Item_Weapon : public Script_Item {
	COMPONENT_ABSTRACT(Script_Item_Weapon, Script_Item)

private:
	Vec3 mDir{};

	bool mDroped{ false };

	int mSign{ 1 };
	float mDeltaTime{};
	float mMaxFloatingSpeed{};

public:
	virtual void Awake() override;
	virtual void Animate() override;

public:
	void SetWeapon(WeaponName weaponName);
	virtual bool InteractOK(Object* user) override;
	virtual ItemType GetItemType() override { return ItemType::Weapon; }
	void Throw(const Vec3& pos);

public:
	virtual void StartOpen();
	virtual void StartDrop();
};