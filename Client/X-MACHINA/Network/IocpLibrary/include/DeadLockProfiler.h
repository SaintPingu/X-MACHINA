#pragma once
#include <stack>
#include <map>
#include <vector>

/*--------------------
	DeadLockProfiler
---------------------*/

#define DEADLOCK_PROFILER DeadLockProfiler::GetInst()

class DeadLockProfiler
{
	DECLARE_SINGLETON(DeadLockProfiler);

private:
	std::vector<int32>	mDiscoveredOrder; // 노드가 발견된 순서를 기록하는 배열
	int32				mDiscoveredCount = 0; // 노드가 발견된 순서
	std::vector<bool>	mIsFinished; // Dfs(i)가 종료 되었는지 여부
	std::vector<int32>	mParent;

public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 index);

private:
	std::unordered_map<const char*, int32>		mNameToID;
	std::unordered_map<int32, const char*>		mIDToName;
	std::map<int32, std::set<int32>>			mLockHistory;

	Mutex _lock;


};

