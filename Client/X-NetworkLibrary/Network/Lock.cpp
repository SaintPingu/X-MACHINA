#include "pch.h"
#include "Lock.h"
#include "ThreadManager.h"



/// +-----------------------------
///          SPIN LOCK
/// -----------------------------+

Lock::SpinLock::SpinLock()
{

}

Lock::SpinLock::~SpinLock()
{
}

void Lock::SpinLock::Lock()
{
    while (mAtomicFlag.test_and_set(std::memory_order_acquire));
}

void Lock::SpinLock::UnLock()
{
    mAtomicFlag.clear(std::memory_order_release);
}


/// +-----------------------------
///            R/W LOCK 
/// -----------------------------+

//DEFINE_SINGLETON(Lock::RWLock);

Lock::RWLock::RWLock()
    : mReaderCount(0), mWriterCount(0) {}

Lock::RWLock::~RWLock()
{
}
void Lock::RWLock::lockRead()
{
    while (true) {
        while (mWriterCount.load() > 0) {
            std::this_thread::yield(); // 기다림
        }
        mReaderCount.fetch_add(1, std::memory_order_acquire);
        if (mWriterCount.load() == 0) {
            break;
        }
        mReaderCount.fetch_sub(1, std::memory_order_release);
    }
}

void Lock::RWLock::unlockRead()
{
    mReaderCount.fetch_sub(1, std::memory_order_release);
}

void Lock::RWLock::lockWrite()
{
    mWriterCount.fetch_add(1, std::memory_order_acquire);
    while (mReaderCount.load() > 0) {
        std::this_thread::yield(); // 기다림
    }
}

void Lock::RWLock::unlockWrite()
{
    mWriterCount.fetch_sub(1, std::memory_order_release);
}




void Lock::Lock::WriteLock(const char* name)
{
    const UINT32 lockThreadId = (mLogFlag.load() & static_cast<UINT32>(LockFlag::Write_Thread_Mask)) >> 16;
    if (TLS_MGR->Get_TlsInfoData()->id == lockThreadId)
    {
        mWriteCount++;
        return;
    }

    const INT64 beginTick = ::GetTickCount64();
    const UINT32 desired = ((TLS_MGR->Get_TlsInfoData()->id << 16) & static_cast<UINT32>(LockFlag::Write_Thread_Mask));
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
            assert(false); /* Time Out */

        std::this_thread::yield(); 
    }
}

void Lock::Lock::WriteUnlock(const char* name)
{
    // ReadLock 다 풀기 전에는 WriteUnlock 불가능.
    if ((mLogFlag.load() & static_cast<UINT32>(LockFlag::Read_Count_Mask)) != 0)
        assert(false); /* "INVALID_UNLOCK_ORDER" */

    const INT32 lockCount = --mWriteCount;
    if (lockCount == 0)
        mLogFlag.store(static_cast<UINT32>(LockFlag::Empty_Mask));
}


void Lock::Lock::ReadLock(const char* name)
{

    const UINT32 lockThreadId = (mLogFlag.load() & static_cast<UINT32>(LockFlag::Write_Thread_Mask)) >> 16;
    if (TLS_MGR->Get_TlsInfoData()->id == lockThreadId)
    {
        mLogFlag.fetch_add(1);
        return;
    }

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
            assert(false); /* Time Out */

        std::this_thread::yield();
    }
}

void Lock::Lock::ReadUnlock(const char* name)
{
    if ((mLogFlag.fetch_sub(1) & static_cast<UINT32>(LockFlag::Read_Count_Mask)) == 0)
       assert(FALSE); /* MULTIPLE_UNLOCK */
}
