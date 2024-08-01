#pragma once


#pragma region Include
#include "AbilityMgr.h"
#include "PheroAbilityInterface.h"
#pragma endregion


#pragma region ClassForwardDecl
class Camera;
class Object;
class GridObject;
class Script_AimController;
class Texture;
#pragma endregion


#pragma region Class
class MindControlAbility : public RenderedAbility, public PheroAbilityInterface {
	using base = RenderedAbility;

private:
	UINT mMaxControlledObjectCnt{};
	UINT mCurrControlledObjectCnt{};

	sptr<Camera> mCamera{};
	
	const float mWindowWidth{};
	const float mWindowHeight{};
	
	// TODO : ���� ���� �����̱� ���ؼ��� �迭�� �����ؾ���
	Object* mPickedTarget{};
	sptr<Script_AimController> mAimController{};

	Vec2 mPrevAimScale{};
	sptr<Texture> mPrevAimTexture{};
	sptr<Texture> mMindControlAimTexture{};

public:
	MindControlAbility();

public:
	virtual void Update(float activeTime) override;
	virtual void Activate() override;
	virtual void DeActivate() override;

	void Click();
	void Terminate();

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

