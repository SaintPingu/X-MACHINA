#pragma once

#pragma region Include
#include "Component/Component.h"
#include "XLManager.h"
#pragma endregion


#pragma region EnumClass
enum class EnemyState : UINT8 {
	Idle = 0,
	Walk,
	GetHit,
	Attack,
	Death,
};
#pragma endregion


#pragma region Struct
struct EnemyStat : public XLData {
	int EnemyLevel{};
	int PheroLevel{};
	float MoveSpeed{};
	float RotationSpeed{};
	float AttackRotationSpeed{};
	float DetectionRange{};
	float AttackRate{};
	float AttackRange{};
	float AttackCoolTime{};
	float MaxHp{};
	std::string AttackAnimName{};
	std::string GetHitAnimName{};
	std::string DeathAnimName{};
};

struct EnemyStatTable : public XLTable {
	EnemyStatTable() : XLTable(true) {}

	virtual sptr<XLData> SetData(const xlnt::range_iterator::reference& row) override {
		sptr<EnemyStat> stat = std::make_shared<EnemyStat>();
		mKeys.push_back(row[0].value<std::string>());
		stat->EnemyLevel = row[1].value<int>();
		stat->PheroLevel = row[2].value<int>();
		stat->MoveSpeed = row[3].value<float>();
		stat->DetectionRange = row[4].value<float>();
		stat->RotationSpeed = row[5].value<float>();
		stat->AttackRotationSpeed = row[6].value<float>();
		stat->AttackRate = row[7].value<float>();
		stat->AttackRange = row[8].value<float>();
		stat->AttackCoolTime = row[9].value<float>();
		stat->MaxHp = row[10].value<float>();
		stat->AttackAnimName = row[11].value<std::string>();
		stat->GetHitAnimName = row[12].value<std::string>();
		stat->DeathAnimName = row[13].value<std::string>();
		return stat;
	}
};
#pragma endregion


#pragma region ClassForwardDecl
class AnimatorController;
#pragma endregion


#pragma region Class
class Script_EnemyManager : public Component {
	COMPONENT(Script_EnemyManager, Component)

public:
	EnemyStat mStat{};
	EnemyState mState = EnemyState::Idle;

	Object* mTarget{};
	Object* mPathTarget{};

	std::stack<Vec3> mPath{};
	sptr<AnimatorController> mController{};

public:
	void Awake() override;
	void Update() override;


public:
	void Reset();
	void RemoveAllAnimation();
	void ForceSetTarget(Object* target) { mTarget = target; }
};
#pragma endregion
