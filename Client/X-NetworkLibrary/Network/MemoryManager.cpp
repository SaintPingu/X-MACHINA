#include "pch.h"
#include "MemoryManager.h"


DEFINE_SINGLETON(MemoryManager);
MemoryManager::MemoryManager()
{
}

MemoryManager::~MemoryManager()
{
    /* Delete SListMemoryPool !!! */
    for (auto& entry : mSLMemPoolsDict_Name) {
        delete entry.second;
        mSLMemPoolsDict_Name[entry.first] = nullptr;

    }
    mSLMemPoolsDict_Name.clear();

    for (auto& entry : mSLMemPoolsDict_Size) {
        SListMemoryPool* ptr = entry.second;
        delete ptr;
        mSLMemPoolsDict_Size[entry.first] = nullptr;
    }
    mSLMemPoolsDict_Size.clear();


}

bool MemoryManager::InitMemories()
{
    /* Memory를 미리 할당해놓자! */

    /* 1. 메모리 사이즈로 관리하는 메모리 풀  */
    int blockNum = 50;
    AddSListMemoryPool(MemorySize::BYTES_32);
    for (int i = 0; i < blockNum; ++i) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_32]->AddMemory();
    }

    AddSListMemoryPool(MemorySize::BYTES_64);
    for (int i = 0; i < blockNum; ++i) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_64]->AddMemory();
    }


    AddSListMemoryPool(MemorySize::BYTES_128);
    for (int i = 0; i < blockNum; ++i) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_128]->AddMemory();
    }


    AddSListMemoryPool(MemorySize::BYTES_256);
    for (int i = 0; i < blockNum; ++i) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_256]->AddMemory();
    }


    AddSListMemoryPool(MemorySize::BYTES_512);
    for (int i = 0; i < blockNum; ++i) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_512]->AddMemory();
    }


    /* 2. 메모리 이름으로 관리하는 메모리 풀 */


    return true;
}

/* 메모리풀을 이름으로 관리 */
void MemoryManager::AddSListMemoryPool(std::string mpName, size_t MemorySize)
{
    /* Locking */
    while (mAtomicFlag.test_and_set(std::memory_order_acquire))
    {
    };
    
    if (mSLMemPoolsDict_Name[mpName] == nullptr) {
        SListMemoryPool* Pool = new SListMemoryPool(MemorySize);
        /* dict (unordered_map) */
        mSLMemPoolsDict_Name[mpName] = Pool;
        mSLMemPoolsDict_Name[mpName]->AddMemory();

    }
    // 해당 메모리 사이즈를 갖는 풀이 이미 존재한다면 
    else {
        mSLMemPoolsDict_Name[mpName]->AddMemory();
    }

    /* UnLocking */
    mAtomicFlag.clear(std::memory_order_release); 
}

/* 메모리 풀을 메모리 사이즈로 관리 */
void MemoryManager::AddSListMemoryPool(MemorySize memSize)
{ 
    /* Locking */
    while (mAtomicFlag.test_and_set(std::memory_order_acquire))
    {
    };

    if (mSLMemPoolsDict_Size[memSize] == nullptr) {
        SListMemoryPool* Pool = new SListMemoryPool(static_cast<size_t>(memSize));
        /* dict (unordered_map) */
        mSLMemPoolsDict_Size[memSize] = Pool;
    }
    // 해당 메모리 사이즈를 갖는 풀이 이미 존재한다면 
    else {
        mSLMemPoolsDict_Size[memSize]->AddMemory();
    }

    /* UnLocking */
    mAtomicFlag.clear(std::memory_order_release);

}

void* MemoryManager::Allocate(size_t size)
{
    while (mAtomicFlag.test_and_set(std::memory_order_acquire)); // Locking
   
    void* ptr = nullptr;

    // Find a suitable memory pool
    for (auto PoolIter : mSLMemPoolsDict_Size) {
        if (size <= PoolIter.second->GetMemorySize()) {
            ptr = PoolIter.second->Pull(); /* Pull Memory Pointer From SListMemoryPool */
            break;
        }
    }
    /* 메모리풀에 없다면 할당 */ 
    if (ptr == nullptr) {
        ptr = ::_aligned_malloc(sizeof(SLIST_ENTRY) + size, MEMORY_ALLOCATION_ALIGNMENT);
        ptr = reinterpret_cast<void*>(reinterpret_cast<char*>(ptr) + sizeof(SLIST_ENTRY));
    }

    mAtomicFlag.clear(std::memory_order_release); // Unlocking
    return ptr;
}

void* MemoryManager::Allocate(std::string name)
{
    while (mAtomicFlag.test_and_set(std::memory_order_acquire)); // Locking

    void* ptr = nullptr;

    // Find a suitable memory pool
    for (auto PoolIter : mSLMemPoolsDict_Name) {
        if (PoolIter.first == name) {
            ptr = PoolIter.second->Pull();
            break;
        }
    }

    mAtomicFlag.clear(std::memory_order_release); // Unlocking
    return ptr;
}

void MemoryManager::Free(std::string mpName, void* ptr)
{
    /* Locking */
    while (mAtomicFlag.test_and_set(std::memory_order_acquire))
    {
    };

    if (mSLMemPoolsDict_Name[mpName] != nullptr) {
        mSLMemPoolsDict_Name[mpName]->Push(ptr);
    }

    /* UnLocking */
    mAtomicFlag.clear(std::memory_order_release);
}

void MemoryManager::Free(size_t size, void* ptr)
{
    ///* Locking */
    while (mAtomicFlag.test_and_set(std::memory_order_acquire))
    {
    };

    if (size <= 32) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_32]->Push(ptr);
    }
    else if (size <= 64) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_64]->Push(ptr);
    }
    else if (size <= 128) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_128]->Push(ptr);

    }
    else if (size <= 256) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_256]->Push(ptr);

    }
    else if (size <= 512) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_512]->Push(ptr);

    }
    else if (size <= 1024) {
        mSLMemPoolsDict_Size[MemorySize::BYTES_1024]->Push(ptr);

    }
  
  /*  if (mSLMemPoolsDict_Size[static_cast<MemorySize>(size)] != nullptr) {
       mSLMemPoolsDict_Size[static_cast<MemorySize>(size)]->Push(ptr);
    }*/
  
    /* UnLocking */
    mAtomicFlag.clear(std::memory_order_release);
}
