#pragma once
#include "Types.h"
#include "NetMemoryPool.h"

/*
	typename Type 전용으로 메모리 풀을 만든다.

	클래스에 하나만 존재하는 static 변수여도
	Type 에따라 다른 클래스이기 때문에
	Type 이 다르면 static 변수값은 다르다. (주소도 다름)

*/

template<typename Type>
class NetObjectPool
{

private:
	static int32		sAllofSize;
	static NetMemoryPool	sPool;

public:
	
	/// +---------------------------
	///		<Type> Memory 생성
	/// ---------------------------+
	template<typename... Args>
	static Type* Pop(Args&&... args)
	{
#ifdef _STOMP
		MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(sAllofSize));
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(ptr, sAllofSize));
#else
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(sPool.Pop(), sAllofSize));
#endif		
		new(memory)Type(std::forward<Args>(args)...); // placement new
		return memory;
	}
	/// +---------------------------
	///		<Type> Memory 소멸
	/// ---------------------------+
	static void Push(Type* obj)
	{
		obj->~Type();
#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
		sPool.Push(MemoryHeader::DetachHeader(obj)); // 메모리 반납 
#endif
	}

	/// +-----------------------------------
	///		Object Pool 전용 shared_ptr
	/// -----------------------------------+
	template<typename... Args>
	static std::shared_ptr<Type> MakeShared(Args&&... args)
	{
		std::shared_ptr<Type> ptr = { Pop(std::forward<Args>(args)...), Push };
		return ptr;
	}

};

template<typename Type>
int32 NetObjectPool<Type>::sAllofSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
NetMemoryPool NetObjectPool<Type>::sPool{ sAllofSize };