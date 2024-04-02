#pragma once

/*----------------
	GlobalQueue
-----------------*/

#define GLOBAL_QUEUE GlobalQueue::GetInst()

class GlobalQueue
{
	DECLARE_SINGLETON(GlobalQueue);

private:
	LockQueue<JobQueueRef> mJobQueues;

public:
	GlobalQueue();
	~GlobalQueue();

	void					Push(JobQueueRef jobQueue);
	JobQueueRef				Pop();


};

