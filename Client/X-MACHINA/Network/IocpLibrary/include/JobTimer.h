#pragma once

struct JobData
{
	JobData(std::weak_ptr<JobQueue> owner, JobRef job) : Owner(owner), Job(job) { };
	
	std::weak_ptr<JobQueue> Owner; // weak_ptr 로 한 이유 : shared_ptr 로 하면 나중에 100 초 예약했을 때 100 초 동안은 소멸이 안되는 상황이 발생한다. 
	JobRef					Job;
};

struct TimerItem
{
	bool operator<(const TimerItem& other) const
	{
		return ExecuteTick > other.ExecuteTick; // 오름차순 
	}

	uint64		ExecuteTick = 0;
	JobData*	jobData     = nullptr; // 생 포인터인 이유는 오리지널 데이터를 레퍼런스 카운트에 영향을 가지 않게 하기 위함이다. 어차피 JobData 는 여기서만 사용하고 해제 할 것이기 때문이다. 

};

#define JOB_TIMER JobTimer::GetInst()
class JobTimer
{
	DECLARE_SINGLETON(JobTimer);

private:
	USE_LOCK;
	NetPriorityQueue<TimerItem>	mItems        = {};
	std::atomic<bool>			mDistributing = false;

public:
	JobTimer();
	~JobTimer();

public:
	void Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner, JobRef job);
	void Distribute(uint64 now); // 배분하다. 
	void Clear();

};

