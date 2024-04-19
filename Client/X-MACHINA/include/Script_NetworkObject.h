#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

#pragma region ClassForwardDecl
class GameObject; 
#pragma endregion




class Script_NetworkObject abstract : public Component {
	COMPONENT_ABSTRACT(Script_NetworkObject, Component)

public:
	virtual void UpdateData(const void* data) abstract;
};




struct Data_Move_GroundPlayer {
	Vec3 Pos{};
	Vec3 Look{};
	Vec3 SpineLook{};
};

struct Data_State_GroundPlayer {
	PlayerMotion PlayerMotion{};
	int WeaponID{};
};

struct Data_GroundPlayer {
	Data_Move_GroundPlayer Transform;
	Data_State_GroundPlayer State;
};


class Script_NetworkObject_GroundPlayer : public Script_NetworkObject {
	COMPONENT(Script_NetworkObject_GroundPlayer, Script_NetworkObject)

private:
	Transform* mSpineBone{};
	float mSpineAngle{};
	std::vector<sptr<GameObject>> mWeapons{};

public:
	virtual void Awake() override;
	virtual void LateUpdate() override;

public:
	virtual void UpdateData(const void* data) override;

private:
	void InitWeapons();
};