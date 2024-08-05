#pragma once


#pragma region Include
#include "Script_Ability.h"
#pragma endregion

class Script_AerialCamera;
class Script_AimController;
class UI;

#pragma region Class
class Script_Ability_AerialController : public Script_RenderedAbility {
	COMPONENT(Script_Ability_AerialController, Script_RenderedAbility)

private:
	UI* mPrevAimUI{};
	UI* mAimUI{};
	Script_AerialCamera* mAerialCamera{};
	Script_AimController* mAimController{};

public:
	virtual void Awake() override;
	virtual void Start() override;

public:
	virtual bool ProcessInput() override;
	virtual bool ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;
	virtual bool ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;

	virtual void On() override;
	virtual void Off() override;

private:
	void ChangeAimToOrigin();
	void ChangeAimToActive();
};
#pragma endregion

