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
	std::vector<int32>	mDiscoveredOrder; // ��尡 �߰ߵ� ������ ����ϴ� �迭
	int32				mDiscoveredCount = 0; // ��尡 �߰ߵ� ����
	std::vector<bool>	mIsFinished; // Dfs(i)�� ���� �Ǿ����� ����
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

