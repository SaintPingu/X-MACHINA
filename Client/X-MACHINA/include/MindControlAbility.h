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
	
	// TODO : 여러 적을 움직이기 위해서는 배열로 관리해야함
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

	// 행동트리 변경
	void ActiveMindControlledEnemyBT();
	void ActivePrevEnemyBT();
	void Terminate();
};
#pragma endregion

