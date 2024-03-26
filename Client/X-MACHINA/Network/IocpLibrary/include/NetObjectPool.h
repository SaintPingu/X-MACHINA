#pragma once
#include "Types.h"
#include "NetMemoryPool.h"

/*
	typename Type �������� �޸� Ǯ�� �����.

	Ŭ������ �ϳ��� �����ϴ� static ��������
	Type ������ �ٸ� Ŭ�����̱� ������
	Type �� �ٸ��� static �������� �ٸ���. (�ּҵ� �ٸ�)

*/

template<typename Type>
class NetObjectPool
{

private:
	static int32		sAllofSize;
	static NetMemoryPool	sPool;

public:
	
	/// +---------------------------
	///		<Type> Memory ����
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
	///		<Type> Memory �Ҹ�
	/// ---------------------------+
	static void Push(Type* obj)
	{
		obj->~Type();
#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
		sPool.Push(MemoryHeader::DetachHeader(obj)); // �޸� �ݳ� 
#endif
	}

	/// +-----------------------------------
	///		Object Pool ���� shared_ptr
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