#pragma once


#pragma region Include
#include "Component/Component.h"
#include "XLManager.h"
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


#pragma region Struct
struct PheroStat : public XLData {
	std::string		ModelName{};
	float			LifeTime{};
	float			Amount{};
};

struct PheroStatTable : public XLTable {
	virtual sptr<XLData> SetData(const xlnt::range_iterator::reference& row) override {
		sptr<PheroStat> stat = std::make_shared<PheroStat>();
		stat->ModelName = row[0].value<std::string>();
		stat->LifeTime = row[1].value<float>();
		stat->Amount = row[2].value<float>();
		return stat;
	}
};
#pragma endregion


#pragma region Class
class Script_Phero : public Component {
	COMPONENT(Script_Phero, Component)

private:
	PheroState mState = PheroState::None;
	sptr<Rigidbody> mRigid{};
	Object* mTarget{};

	PheroStat mStat{};
	
	float mCurrTime{};
	float mPickupRange{};
	float mFllowSpeed{};
	Vec3 mSpreadDest{};
	Vec3 mSpreadStart{};

public:
	void Start() override;
	void Update() override;
	void LateUpdate() override;

public:
	const PheroStat& GetPheroStat() { return mStat; }
	UINT32 GetID() const { return mObject->GetID(); }

public:
	void SetPheroStat(int level);
	void SetID(int monster_id, int phero_index);

private:
	bool IntersectTerrain();
	bool SpreadAllDirections();
	void FloatGently();
	bool CheckPlayerRange();
	void FollowToTarget();
};
#pragma endregion

