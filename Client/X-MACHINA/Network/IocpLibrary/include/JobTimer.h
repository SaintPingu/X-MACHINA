#pragma once

struct JobData
{
	JobData(std::weak_ptr<JobQueue> owner, JobRef job) : Owner(owner), Job(job) { };
	
	std::weak_ptr<JobQueue> Owner; // weak_ptr �� �� ���� : shared_ptr �� �ϸ� ���߿� 100 �� �������� �� 100 �� ������ �Ҹ��� �ȵǴ� ��Ȳ�� �߻��Ѵ�. 
	JobRef					Job;
};

struct TimerItem
{
	bool operator<(const TimerItem& other) const
	{
		return ExecuteTick > other.ExecuteTick; // �������� 
	}

	uint64		ExecuteTick = 0;
	JobData*	jobData     = nullptr; // �� �������� ������ �������� �����͸� ���۷��� ī��Ʈ�� ������ ���� �ʰ� �ϱ� �����̴�. ������ JobData �� ���⼭�� ����ϰ� ���� �� ���̱� �����̴�. 

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
	void Distribute(uint64 now); // ����ϴ�. 
	void Clear();

};

