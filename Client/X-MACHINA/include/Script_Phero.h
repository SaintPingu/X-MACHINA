#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class InstObject;
class Rigidbody;
#pragma endregion


#pragma region EnumClass
enum class PheroState : UINT8 {
	None = 0,
	Spread,
	Idle,
	Follow,
};
#pragma endregion


#pragma region EnumClass
struct PheroStat {
	float LifeTime{};
	float Amount{};
	float Level{};
};
#pragma endregion


#pragma region Class
class Script_Phero : public Component {
	COMPONENT(Script_Phero, Component)

private:
	PheroState mState = PheroState::None;
	sptr<Rigidbody> mRigid{};
	sptr<Object> mTarget{};

	PheroStat mStat{};
	
	float mCurrTime{};
	float mPickupRange{};
	float mFllowSpeed{};
	Vec3 mSpreadDest{};

public:
	void Start() override;
	void Update() override;
	void LateUpdate() override;

public:
	void SetPheroLevel();

private:
	bool IntersectTerrain();
	void FloatGently();
	bool CheckPlayerRange();
	void FollowToTarget();
};
#pragma endregion

