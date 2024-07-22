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
struct UITexture;
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

	UITexture* mPrevUITexture{};
	sptr<UITexture> mMindControlAimUITexture{};

public:
	MindControlAbility();

public:
	virtual void Update(float activeTime) override;
	virtual void Activate() override;
	virtual void DeActivate() override;

protected:
	virtual bool ReducePheroAmount(bool checkOnly = false) override;

private:
	Object* PickingObject(const Vec2& screenPos);

	// �ൿƮ�� ����
	void ActiveMindControlledEnemyBT();
	void ActivePrevEnemyBT();
	void Terminate();
};
#pragma endregion

