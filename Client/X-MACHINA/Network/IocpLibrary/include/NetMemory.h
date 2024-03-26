#pragma once
#include "Allocator.h"

#define MEMORY NetMemory::GetInst()
class NetMemoryPool;

#pragma region MEMORY INFO ENUM CLASS
enum class MemoryInfo
{
	/// BYTE   ↓ 0			   ↓ 1024			  ↓ 2048	    ↓ 4096
/// MEMORY [32][64]...[1024] [1152][1280]...[2048] [256][256][256]
/// INDEX  0   1       
///        ↑		
///        ↑
///	  mPoolTable[0]

	PoolCount = (1024 / 32) + (1024 / 128) + (2048 / 256),
	MaxAllocSize = 4096,
	_CountOf,
};
#pragma endregion


class NetMemory
{
	DECLARE_SINGLETON(NetMemory);

private:
	std::vector<NetMemoryPool*> mMemoryPools;

	// 메모리 크기 <-> 메모리 풀
	// O(1) 빠르게 찾기 위한 테이블
	NetMemoryPool* mPoolTable[static_cast<uint32>(MemoryInfo::MaxAllocSize) + 1];

public:
	NetMemory();
	~NetMemory();

	void*	Allocate(int32 size);
	void	Release(void* ptr);


};

/// +----------------------------
///		MEMORY POOL ALLOCATOR 
///	----------------------------+
template<typename Type, typename... Args>
Type* Xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
	new(memory)Type(std::forward<Args>(args)...); // placement new
	return memory;
}

template<typename Type>
void Xdelete(Type* obj)
{
	obj->~Type();
	PoolAllocator::Release(obj);
}

template<typename Type, typename... Args>
std::shared_ptr<Type> MakeShared(Args&&... args)
{
	return std::shared_ptr<Type>{ Xnew<Type>(std::forward<Args>(args)...), Xdelete<Type> };
}