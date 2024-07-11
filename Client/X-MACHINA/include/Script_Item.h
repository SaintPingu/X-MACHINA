#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

class GameObject;


class Script_Item_WeaponCrate : public Component , SceneScript {
	COMPONENT_ABSTRACT(Script_Item_WeaponCrate, Component)

public:
	WeaponName mWeaponName;
	Transform* mCap{};

	float mCapPitch{};
	bool mIsOpend{};

public:
	virtual void Awake() override;
	virtual void Animate() override;

public:
	virtual void LoadData(rsptr<ScriptExporter> exporter) override;
	virtual WeaponName Interact();
};