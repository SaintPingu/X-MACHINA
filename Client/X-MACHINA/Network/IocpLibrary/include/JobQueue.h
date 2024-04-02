#pragma once
#include "Job.h"
#include "LockQueue.h"
#include "JobTimer.h"

/// +------------------
///		 JOB QUEUE 
/// ------------------+

class JobQueue : public std::enable_shared_from_this<JobQueue>
{

protected:
	LockQueue<JobRef>		mJobs;
	Atomic<int32>			mJobCount = 0;


public:
	inline void DoAsync(CallbackType&& callback)
	{
		Push(NetObjectPool<Job>::MakeShared(std::move(callback)));
	}

	template<typename T, typename Ret, typename... Args>
	inline void DoAsync(Ret(T::*memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		Push(NetObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	}

public:
	// tickAfter 시간 후에 시작 ( 예약 시스템 ) 
	inline void DoTimer(uint64 tickAfter, CallbackType&& callback)
	{
		JobRef job = NetObjectPool<Job>::MakeShared(std::move(callback));
		JOB_TIMER->Reserve(tickAfter, shared_from_this()/*owner*/, job); // ( 예약 )
	}

	// tickAfter 시간 후에 시작 ( 예약 시스템 ) 
	template<typename T, typename Ret, typename... Args>
	inline void DoTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		JobRef job = NetObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		JOB_TIMER->Reserve(tickAfter, shared_from_this(), job);
	}
public:
	void Push(JobRef job, bool pushOnly = false);
	void Execute();
	void ClearJobs() { mJobs.Clear(); }



};

