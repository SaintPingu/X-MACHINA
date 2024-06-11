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

			sptr<PheroObjectStat> stat = std::make_shared<PheroObjectStat>();
			stat->MaxPheroCount = row[0].value<int>();
			stat->LevelPerPDRs.push_back(row[1].value<int>());
			stat->LevelPerPDRs.push_back(row[2].value<int>());
			stat->LevelPerPDRs.push_back(row[3].value<int>());
			mDatas.push_back(stat);
		}
	}
};
#pragma endregion


#pragma region Class
class Script_PheroObject : public Component {
	COMPONENT(Script_PheroObject, Component)

private:
	int mPheroObjectLevel{};
	PheroObjectStat mStartStat{};
	PheroObjectLastStat mLastStat{};

public:
	virtual void Start() override;
	virtual void OnDestroy() override;

public:
	void SetPheroObjectLevel(int level) { mPheroObjectLevel = level; }

private:
	void GenerateRandomPheroCount();
	void GeneratePheroPool(int pheroLevel, int pheroCount);
};
#pragma endregion
