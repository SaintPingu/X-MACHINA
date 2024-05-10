#pragma once

/// +-----------------------------------------------
///					  MemoryManager 
/// ________________________________________________
///	구조
///					
/// ----------------------------------------------+


#include "SListMemoryPool.h"

enum class MemorySize : UINT16
{
	BYTES_32   = 32,
	BYTES_64   = 64,
	BYTES_128  = 128,
	BYTES_256  = 256,
	BYTES_512  = 512, 
	BYTES_1024 = 1024,

};

struct SListMemoryPoolInfo {
	std::string SListMemoryPoolName	= {};
	size_t		MemorySize		= {};
};

#define MEMORY MemoryManager::GetInst()
using Memory = class MemoryManager;
class MemoryManager
{
	DECLARE_SINGLETON(MemoryManager);

private:
	//std::vector<SListMemoryPool*>	mSListMemoryPools = {};
	std::atomic_flag				mAtomicFlag       = ATOMIC_FLAG_INIT; // Atomic flag for thread safety

	std::unordered_map<std::string, SListMemoryPool*> mSLMemPoolsDict_Name = {};
	std::unordered_map<MemorySize, SListMemoryPool*>  mSLMemPoolsDict_Size = {};

public:
	MemoryManager();
	~MemoryManager();

public:
	bool InitMemories();


	void  AddSListMemoryPool(std::string mpName, size_t MemorySize);
	void  AddSListMemoryPool(MemorySize memSize);

	void* Allocate(size_t size);
	void* Allocate(std::string name);
	
	void  Free(std::string mpName, void* ptr);  // 이름으로 저장한 메모리풀에 ptr 을 넣는다. 
	void  Free(size_t size, void* ptr);			// 사이즈로 저장한 메모리풀에 ptr 을 넣는다. 



public:
	template<typename Type, typename... Args>
	static Type* New(Args&&... args);
	template<typename Type>
	static void Delete(Type* ptr);
	template<typename Type, typename... Args>
	static std::shared_ptr<Type> Make_Shared(Args&&... args);

};

template<typename Type, typename ...Args>
inline Type* MemoryManager::New(Args && ...args)
{
	Type* Memory = static_cast<Type*>(MEMORY->Allocate(sizeof(Type)));
	new(Memory)Type(std::forward<Args>(args)...); /* Placement New */
	return Memory;;
}

template<typename Type>
inline void MemoryManager::Delete(Type* ptr)
{
	ptr->~Type();
	MEMORY->Free(static_cast<size_t>(sizeof(Type)), ptr);
	//MEMORY->Delete(ptr);
}

template<typename Type, typename ...Args>
inline std::shared_ptr<Type> MemoryManager::Make_Shared(Args&&... args)
{
	return std::shared_ptr<Type>{ New<Type>(std::forward<Args>(args)...), Delete<Type>};
}
