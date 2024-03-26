#pragma once


/*
	InterlockedPushEntrySList 를 사용하기 위해서 SLIST_ENTRY 형태로 만듦

	SLIST ( InterLocked Singly Linked List )
	MS 에서 제공하는 연결리스트로 atomic operation 을 지원한다.
	메모리 할당 시 _aligned_malloc 을 사용해야한다.

	SLIST_HEADER : SList 의 헤더
	SLIST_ENTRY  : SList 의 엔트리

*/

DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
/*
	↓ (header)	  ↓ (++header)
	[MemoryHeader][Data...]
*/
#pragma region Variable
	int32 AllocSize;
#pragma endregion

#pragma region Func
	/// [MemoryHeader][Data]
	MemoryHeader(int32 size) : AllocSize(size) { }
	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		/// placement new는 주로 메모리를 미리 할당하고자 할 때, 
		/// 특정 메모리 위치에 객체를 직접 생성하고자 할 때 사용됩니다. 
		/// 예를 들어, 메모리 풀이나 메모리 매니저를 사용할 때 유용하게 활용될 수 있습니다.

		new(header)MemoryHeader(size); /// placement new --> header 메모리 위치에 MemoryHeader를 size 크기 만큼 데이터를 만든다.
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}
#pragma endregion
};



DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) // 16 바이트 정렬 SLIST 를 사용하기 위해서는 16 바이트 정렬을 해주어야한다. (= 객체 할당했을 때 그 주소가 16으로 나뉘어야 함)
class NetMemoryPool
{
private:
	SLIST_HEADER		mHeader;
	int32				mAllocSize    = 0;
	std::atomic<int32>	mUseCount     = 0;
	std::atomic<int32>	mReserveCount = 0;

public:
	NetMemoryPool(int32 allocSize);
	~NetMemoryPool();

	void		  Push(MemoryHeader* ptr);	/// 메모리를 반납하겠다. 
	MemoryHeader* Pop();					/// 메모리를 받아와서 사용하겠다.


};

