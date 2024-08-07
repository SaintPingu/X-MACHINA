#pragma once


#pragma region Include
#include "Component/Component.h"
#include "XLManager.h"
#pragma endregion


#pragma region ClassForwardDecl
class ObjectPool;
#pragma endregion


#pragma region Struct
struct PheroObjectStat : public XLData {
	int MaxPheroCount{};			// Phero create max count
	std::vector<int> LevelPerPDRs;	// level per Phero drop rate
};

struct PheroObjectLastStat {
	int PheroCount{};				// Phero create count
	std::vector<int> LevelPerPDCs;	// level per Phero drop count
};

struct PheroObjectStatTable : public XLTable {
	virtual sptr<XLData> SetData(const xlnt::range_iterator::reference& row) override {
		sptr<PheroObjectStat> stat = std::make_shared<PheroObjectStat>();
		stat->MaxPheroCount = row[0].value<int>();
		stat->LevelPerPDRs.push_back(row[1].value<int>());
		stat->LevelPerPDRs.push_back(row[2].value<int>());
		stat->LevelPerPDRs.push_back(row[3].value<int>());
		return stat;
	}
};
#pragma endregion


#pragma region Class
class Script_PheroObject : public Component {
	COMPONENT(Script_PheroObject, Component)

private:
	PheroObjectStat mStartStat{};
	PheroObjectLastStat mLastStat{};

public:
	virtual void Start() override;
	virtual void OnDestroy() override;

public:
	static std::vector<class Script_Phero*> GeneratePheroPool(int pheroLevel, int pheroCount, const Vec3& position, int monsterID = 0);

private:
	void GenerateRandomPheroCount();
	void GeneratePheroPool(int pheroLevel, int pheroCount);
};
#pragma endregion
