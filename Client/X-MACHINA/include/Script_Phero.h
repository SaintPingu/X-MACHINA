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
public:
	virtual void Load(const std::string& path) override {
		xlnt::workbook wb;
		wb.load(path);
		auto ws = wb.active_sheet();
		
		// 첫 행을 빼기 위해서 더미 값을 넣어준다.
		mDatas.resize(1);
		bool first_row_skipped = false;

		for (auto row : ws.rows(false)) {
			if (!first_row_skipped) {
				first_row_skipped = true;
				continue;
			}

			sptr<PheroStat> stat = std::make_shared<PheroStat>();
			stat->ModelName = row[0].value<std::string>();
			stat->LifeTime = row[1].value<float>();
			stat->Amount = row[2].value<float>();
			mDatas.push_back(stat);
		}
	}
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
	const PheroStat& GetPheroStat() { return mStat; }
	void SetPheroStat(int level);

private:
	bool IntersectTerrain();
	void FloatGently();
	bool CheckPlayerRange();
	void FollowToTarget();
};
#pragma endregion

