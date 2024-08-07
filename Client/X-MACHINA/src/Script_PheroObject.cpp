#include "stdafx.h"
#include "Script_PheroObject.h"

#include "BattleScene.h"
#include "ObjectPool.h"
#include "Object.h"
#include "XLManager.h"

#include "Script_Phero.h"
#include "Script_EnemyManager.h"

#include "ClientNetwork/Contents/ClientNetworkManager.h"


void Script_PheroObject::Start()
{
	base::Start();
	
	int level = 1;
	auto& enemyMgr = mObject->GetComponent<Script_EnemyManager>();
	if (enemyMgr) {
		level = enemyMgr->mStat.PheroLevel;
	}

	XLManager::I->Set(level, mStartStat);

	GenerateRandomPheroCount();
}

void Script_PheroObject::OnDestroy()
{
	for (int i = 1; i <= mLastStat.LevelPerPDCs.size(); ++i) {
		GeneratePheroPool(i, mLastStat.LevelPerPDCs[i - 1]);
	}

	base::OnDestroy();
}

void Script_PheroObject::GenerateRandomPheroCount()
{
	mLastStat.PheroCount = Math::RandInt(1, mStartStat.MaxPheroCount);
	mLastStat.LevelPerPDCs.resize(mStartStat.LevelPerPDRs.size());

	for (int i = 0; i < mLastStat.PheroCount; ++i) {
		int randValue = Math::RandInt(0, 100);
		float cumulativeProbability = 0.f;

		for (int j = 0; j < mStartStat.LevelPerPDRs.size(); ++j) {
			cumulativeProbability += mStartStat.LevelPerPDRs[j];
			if (randValue <= cumulativeProbability) {
				mLastStat.LevelPerPDCs[j]++;
				break;
			}
		}
	}
}

void Script_PheroObject::GeneratePheroPool(int pheroLevel, int pheroCount)
{
	GeneratePheroPool(pheroLevel, pheroCount, mObject->GetPosition());
}

std::vector<Script_Phero*> Script_PheroObject::GeneratePheroPool(int pheroLevel, int pheroCount, const Vec3& position, int monsterID)
{
	std::vector<Script_Phero*> pheros;
	if (pheroCount < 1 || pheroLevel < 1) {
		return pheros;
	}

	auto& pheroPool = BattleScene::I->CreateObjectPool("Level1Phero", pheroCount, [&](rsptr<InstObject> object) {
		object->SetTag(ObjectTag::Dynamic);
		});

	std::string modelName{};
	int idx{};
	for (auto& phero : pheroPool->GetMulti(pheroCount, true))
	{
		phero->SetPosition(position);
		auto& script = phero->AddComponent<Script_Phero>();
		script->SetPheroStat(pheroLevel);
		script->SetID(monsterID, idx++);
		pheros.emplace_back(script.get());

		if (modelName.empty()) {
			modelName = script->GetPheroStat().ModelName;
		}
	}


	pheroPool->ChangeModel(modelName);

	return pheros;
}
