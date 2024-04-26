#pragma once

#include <thread>
#include <functional>

/*------------------
	ThreadManager
-------------------*/

#define THREAD_MGR ThreadManager::GetInst()

class ThreadManager
{
	DECLARE_SINGLETON(ThreadManager);

private:
	std::mutex					mLock;
	std::vector<std::thread>	mThreads;
public:
	ThreadManager();
	~ThreadManager();

	void	Launch(std::function<void(void)> callback);
	void	Join();

	static void InitTLS();
	static void DestroyTLS();

	static void DoGlobalQueueWork();
	static void DistributeReservedJobs();


};

