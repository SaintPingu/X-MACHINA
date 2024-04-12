#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
#pragma endregion


#pragma region Class
class Script_LiveObject abstract : public Component {
	COMPONENT_ABSTRACT(Script_LiveObject, Component)

private:
	float mMaxHP{};
	float mCrntHP{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

	void SetMaxHP(float hp) { mMaxHP = hp; }

	virtual void Hit(float damage);
	virtual void Dead();
};

#pragma endregion