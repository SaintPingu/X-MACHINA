#pragma once

#define USE_MANY_LOCKS(count)	Lock::Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define	READ_LOCK_IDX(idx)		Lock::ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name());
#define READ_LOCK_SCOPE			READ_LOCK_IDX(0)
#define	WRITE_LOCK_IDX(idx)		Lock::WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());
#define WRITE_LOCK_SCOPE		WRITE_LOCK_IDX(0)


namespace Lock
{
    /* SpinLock */
    class SpinLock {
    private:
        std::atomic_flag  mAtomicFlag = ATOMIC_FLAG_INIT;
    public:
        SpinLock();
        ~SpinLock();
        void Lock();
        void UnLock();
    };

    //#define RWLOCK RWLock::GetInst()
    class RWLock {
    private:
        // DECLARE_SINGLETON(RWLock);

    private:
        std::atomic<int> mReaderCount;
        std::atomic<int> mWriterCount;

    public:
        RWLock();
        ~RWLock();
        void lockRead();
        void unlockRead();
        void lockWrite();
        void unlockWrite();

    };

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

    /// +---------------------------------
    ///  SRWLock - Slim Read Write Lock
    /// ---------------------------------+
    class SRWLock
    {
        typedef enum { NOT_INIT, UNLOCKED, READLOCKED, WRITELOCKED } MASK;
    private:
        SRWLOCK mSrwLock; // MSDN 
        MASK lockMask = NOT_INIT;

    public:
        SRWLock() { InitializeSRWLock(&mSrwLock); }

        /* Read Lock - Unlock */
        void LockRead() {
            AcquireSRWLockShared(&mSrwLock);
        }
        void UnlockRead() {
            ReleaseSRWLockShared(&mSrwLock);
        }

        /* Write Lock - Unlock */
        void LockWrite() {
            AcquireSRWLockExclusive(&mSrwLock);
        }
        void UnlockWrite() {
            ReleaseSRWLockExclusive(&mSrwLock);
        }

        // �б� ����� ȹ���� �������� Ȯ��
        bool TryReadLocked() const {
            return TryAcquireSRWLockShared(const_cast<SRWLOCK*>(&mSrwLock)) == TRUE;
        }
        bool TryWriteLocked() const {
            return TryAcquireSRWLockExclusive(const_cast<SRWLOCK*>(&mSrwLock)) == TRUE;
        }

    };

    class SRW_WriteLockGaurd
    {
    private:
        SRWLock& _lock;
    public:
        SRW_WriteLockGaurd(SRWLock& lock) : _lock(lock) { _lock.LockWrite(); }
        ~SRW_WriteLockGaurd() { _lock.UnlockWrite(); };

    };
#define WriteLockScope(srwLock) SRW_WriteLockGuard(srwLock)

}

