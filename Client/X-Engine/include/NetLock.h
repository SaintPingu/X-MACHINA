#pragma once
#pragma once
/*
    1. 재귀적으로 Lock 을 잡기위해서 생성 ( 표준 뮤텍스는 재귀적으로 락을 잡을 수 없다 )

    Read  -> 자유롭게 접근
    Write -> 배타적으로 접근

    WriteLock 을 한 상태에서 재귀적으로 또 WriteLock 을 할 때
    크래쉬를 내는 것이 아니라 mWriteCount 를 늘려서 한 번 더 Lock 을 잡게 허락한다.


    WriteLock --> WriteLock ( O )
    WriteLock --> ReadLock  ( O )
    ReadLock  --> WriteLock ( X )

*/



/// +----------------------------
///			RW SPIN LOCK
/// ----------------------------+

/*--------------------------------------------
[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
W : WriteFlag (Exclusive Lock Owner ThreadId)
R : ReadFlag (Shared Lock Count)
---------------------------------------------*/
/*---------------
      Lock
---------------*/

#define USE_MANY_LOCKS(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define	READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name());
#define READ_LOCK				READ_LOCK_IDX(0)
#define	WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_IDX(0)

/*---------------
      Crash
---------------*/

#define CRASH(cause)						\
{											\
	UINT32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

#define ASSERT_CRASH(expr)			\
{									\
	if (!(expr))					\
	{								\
		CRASH("ASSERT_CRASH");		\
		__analysis_assume(expr);	\
	}								\
}
enum class LockFlag : UINT32
{
    Full_Bit_Size = 32,
    Half_Bit_Size = 16,

    Acquire_Timeout_Tick = 10'000,
    Max_Spin_Count = 5'000,

    Write_Thread_Mask = 0xFFFF'0000, /// Thread ID 
    Read_Count_Mask = 0x0000'FFFF, /// Read Count 
    Empty_Mask = 0x0000'0000,

    _count,
};

class Lock
{
private:
    std::atomic<UINT32> mLogFlag = static_cast<UINT32>(LockFlag::Empty_Mask);
    UINT16              mWriteCount = 0;



public:
    void WriteLock(const char* name);
    void WriteUnlock(const char* name);
    void ReadLock(const char* name);
    void ReadUnlock(const char* name);


};

/*----------------
    LockGuards
-----------------*/

class ReadLockGuard
{
public:
    ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.ReadLock(name); }
    ~ReadLockGuard() { _lock.ReadUnlock(_name); }

private:
    Lock& _lock;
    const char* _name;
};

class WriteLockGuard
{
public:
    WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.WriteLock(name); }
    ~WriteLockGuard() { _lock.WriteUnlock(_name); }

private:
    Lock& _lock;
    const char* _name;
};