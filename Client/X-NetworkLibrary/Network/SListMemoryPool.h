#pragma once


/// +-----------------------------------------------
///					  MemoryPool 
/// ________________________________________________
///				       메모리 풀  
/// 
/// Thread Safe 메모리 풀 
/// ----------------------------------------------+


DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) /* 16바이트 메모리 정렬 */
struct SListMemoryBlock 
{
    SLIST_ENTRY entry = {}; // SLIST 엔트리
};

DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) /* 16바이트 메모리 정렬 */
class SListMemoryPool {

private:
    SLIST_HEADER                mSListHeader  = {};       

    size_t                      mMemorySize   = {};
    std::atomic<size_t>         mNumBlocks    = {};      // 할당할 메모리 블록의 크기
    
    std::atomic<size_t>         mPushCount = {};      // 할당할 메모리 블록의 크기
    std::atomic<size_t>         mPullCount = {};      // 할당할 메모리 블록의 크기

public:
    SListMemoryPool(size_t MemorySize);
    ~SListMemoryPool();

public:
    void*   Pull();
    void    Push(void* ptr);
    void    AddMemory();


    size_t GetNumBlocks() const { return mNumBlocks.load(); }
    size_t GetMemorySize() const { return mMemorySize; }

};