#pragma once


#pragma region Include
#include "Script_Ability.h"
#include "PheroAbilityInterface.h"
#pragma endregion

class Object;
class Texture;
class Script_AimController;

#pragma region Class
class Script_Ability_MindControl : public Script_RenderedAbility, public PheroAbilityInterface {
	COMPONENT(Script_Ability_MindControl, Script_RenderedAbility)

private:
	sptr<class Script_PheroPlayer> mPlayer{};

	UINT mMaxControlledObjectCnt{};
	UINT mCurrControlledObjectCnt{};

	// TODO : ���� ���� �����̱� ���ؼ��� �迭�� �����ؾ���
	Object* mPickedTarget{};
	sptr<Script_AimController> mAimController{};

	Vec2 mPrevAimScale{};
	sptr<Texture> mPrevAimTexture{};
	sptr<Texture> mMindControlAimTexture{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void On() override;
	virtual void Off() override;

	void Click();

protected:
	virtual bool ReducePheroAmount(bool checkOnly = false) override;

private:
	Object* PickingObject(const Vec2& screenPos);

	// �ൿƮ�� ����
	void ActiveMindControlledEnemyBT();
	void ActivePrevEnemyBT();

	void ChangeAimToOrigin();
	void ChangeAimToActive();
};
#pragma endregion

