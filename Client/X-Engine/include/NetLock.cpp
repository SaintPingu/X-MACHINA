#include "EnginePch.h"
#include "NetLock.h"
#include "TLS.h"
void Lock::WriteLock(const char* name)
{

	// 동일한 쓰레드가 소유하고 있다면 무조건 성공.
	const UINT32 lockThreadId = (mLogFlag.load() & static_cast<UINT32>(LockFlag::Write_Thread_Mask)) >> 16;
	if (TLS_Engine_ThreadID == lockThreadId)
	{
		mWriteCount++;
		return;
	}

	// 아무도 소유 및 공유하고 있지 않을 때, 경합해서 소유권을 얻는다.
	const INT64 beginTick = ::GetTickCount64();
	const UINT32 desired = ((TLS_Engine_ThreadID << 16) & static_cast<UINT32>(LockFlag::Write_Thread_Mask));
	while (true)
	{
		for (UINT32 spinCount = 0; spinCount < static_cast<UINT32>(LockFlag::Max_Spin_Count); spinCount++)
		{
			UINT32 expected = static_cast<UINT32>(LockFlag::Empty_Mask);
			if (mLogFlag.compare_exchange_strong(OUT expected, desired))
			{
				mWriteCount++;
				return;
			}
		}

		if (::GetTickCount64() - beginTick >= static_cast<UINT32>(LockFlag::Acquire_Timeout_Tick))
			CRASH("LOCK_TIMEOUT");

		std::this_thread::yield(); // 다른 스레드에게 실행 시간을 양보한다. 
		// - 자기가 할일ㅇ의 우선순위가 낮을 경우
		// - 작업을 끝내고 다음 작업의 도착을 기다리는 경우 : 루프를 돌면서 작업의 도착을 확인하고 yield 해야한다. 
	}
}

void Lock::WriteUnlock(const char* name)
{

	// ReadLock 다 풀기 전에는 WriteUnlock 불가능.
	if ((mLogFlag.load() & static_cast<UINT32>(LockFlag::Read_Count_Mask)) != 0)
		CRASH("INVALID_UNLOCK_ORDER");

	const INT32 lockCount = --mWriteCount;
	if (lockCount == 0)
		mLogFlag.store(static_cast<UINT32>(LockFlag::Empty_Mask));
}

void Lock::ReadLock(const char* name)
{

	// 동일한 쓰레드가 소유하고 있다면 무조건 성공.
	const UINT32 lockThreadId = (mLogFlag.load() & static_cast<UINT32>(LockFlag::Write_Thread_Mask)) >> 16;
	if (TLS_Engine_ThreadID == lockThreadId)
	{
		mLogFlag.fetch_add(1);
		return;
	}

	// 아무도 소유하고 있지 않을 때 경합해서 공유 카운트를 올린다.
	const INT64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (UINT32 spinCount = 0; spinCount < static_cast<UINT32>(LockFlag::Max_Spin_Count); spinCount++)
		{
			UINT32 expected = (mLogFlag.load() & static_cast<UINT32>(LockFlag::Read_Count_Mask));
			if (mLogFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		if (::GetTickCount64() - beginTick >= static_cast<UINT32>(LockFlag::Acquire_Timeout_Tick))
			CRASH("LOCK_TIMEOUT");

		std::this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{

	if ((mLogFlag.fetch_sub(1) & static_cast<UINT32>(LockFlag::Read_Count_Mask)) == 0)
		CRASH("MULTIPLE_UNLOCK");
}
