#include "EnginePch.h"
#include "NetLock.h"
#include "TLS.h"
void Lock::WriteLock(const char* name)
{

	// ������ �����尡 �����ϰ� �ִٸ� ������ ����.
	const UINT32 lockThreadId = (mLogFlag.load() & static_cast<UINT32>(LockFlag::Write_Thread_Mask)) >> 16;
	if (TLS_Engine_ThreadID == lockThreadId)
	{
		mWriteCount++;
		return;
	}

	// �ƹ��� ���� �� �����ϰ� ���� ���� ��, �����ؼ� �������� ��´�.
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

		std::this_thread::yield(); // �ٸ� �����忡�� ���� �ð��� �纸�Ѵ�. 
		// - �ڱⰡ ���Ϥ��� �켱������ ���� ���
		// - �۾��� ������ ���� �۾��� ������ ��ٸ��� ��� : ������ ���鼭 �۾��� ������ Ȯ���ϰ� yield �ؾ��Ѵ�. 
	}
}

void Lock::WriteUnlock(const char* name)
{

	// ReadLock �� Ǯ�� ������ WriteUnlock �Ұ���.
	if ((mLogFlag.load() & static_cast<UINT32>(LockFlag::Read_Count_Mask)) != 0)
		CRASH("INVALID_UNLOCK_ORDER");

	const INT32 lockCount = --mWriteCount;
	if (lockCount == 0)
		mLogFlag.store(static_cast<UINT32>(LockFlag::Empty_Mask));
}

void Lock::ReadLock(const char* name)
{

	// ������ �����尡 �����ϰ� �ִٸ� ������ ����.
	const UINT32 lockThreadId = (mLogFlag.load() & static_cast<UINT32>(LockFlag::Write_Thread_Mask)) >> 16;
	if (TLS_Engine_ThreadID == lockThreadId)
	{
		mLogFlag.fetch_add(1);
		return;
	}

	// �ƹ��� �����ϰ� ���� ���� �� �����ؼ� ���� ī��Ʈ�� �ø���.
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
