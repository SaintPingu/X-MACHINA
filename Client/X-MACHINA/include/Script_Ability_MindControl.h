#pragma once


#pragma region Include
#include "Script_Ability.h"
#include "PheroAbilityInterface.h"
#pragma endregion

class Object;
class Texture;
class Script_AimController;
class CooldownCircleUI;

#pragma region Class
class Script_Ability_MindControl : public Script_RenderedAbility, public PheroAbilityInterface, public IconAbilityInterface{
	COMPONENT(Script_Ability_MindControl, Script_RenderedAbility)

protected:
	Object* mPickedTarget{};

private:
	sptr<class Script_PheroPlayer> mPlayer{};

	UINT mMaxControlledObjectCnt{};
	UINT mCurrControlledObjectCnt{};

	sptr<Script_AimController> mAimController{};
	sptr<CooldownCircleUI> mRemainActiveTimeUI{};

	Vec2 mPrevAimScale{};
	sptr<Texture> mPrevAimTexture{};
	sptr<Texture> mMindControlAimTexture{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual bool ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;

public:
	virtual void On() override;
	virtual void Off() override;

protected:
	virtual bool ReducePheroAmount(bool checkOnly = false) override;

private:
	Object* PickingObject(const Vec2& screenPos);
	void Click();
	void ChangeAimToOrigin();
	void ChangeAimToActive();
};

class Script_Remote_Ability_MindControl : public Script_Ability_MindControl {
	COMPONENT(Script_Remote_Ability_MindControl, Script_Ability_MindControl)

public:
	virtual void On() override;
	virtual void Off() override;
};
#pragma endregion

