#pragma once


/// +-----------------------------------------------
///					  MemoryPool 
/// ________________________________________________
///				       �޸� Ǯ  
/// 
/// Thread Safe �޸� Ǯ 
/// ----------------------------------------------+


DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) /* 16����Ʈ �޸� ���� */
struct SListMemoryBlock 
{
    SLIST_ENTRY entry = {}; // SLIST ��Ʈ��
};

DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) /* 16����Ʈ �޸� ���� */
class SListMemoryPool {

private:
    SLIST_HEADER                mSListHeader  = {};       

    size_t                      mMemorySize   = {};
    std::atomic<size_t>         mNumBlocks    = {};      // �Ҵ��� �޸� ����� ũ��
    
    std::atomic<size_t>         mPushCount = {};      // �Ҵ��� �޸� ����� ũ��
    std::atomic<size_t>         mPullCount = {};      // �Ҵ��� �޸� ����� ũ��

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