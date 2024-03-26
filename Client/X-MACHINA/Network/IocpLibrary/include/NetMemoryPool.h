#pragma once


/*
	InterlockedPushEntrySList �� ����ϱ� ���ؼ� SLIST_ENTRY ���·� ����

	SLIST ( InterLocked Singly Linked List )
	MS ���� �����ϴ� ���Ḯ��Ʈ�� atomic operation �� �����Ѵ�.
	�޸� �Ҵ� �� _aligned_malloc �� ����ؾ��Ѵ�.

	SLIST_HEADER : SList �� ���
	SLIST_ENTRY  : SList �� ��Ʈ��

*/

DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
/*
	�� (header)	  �� (++header)
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
		/// placement new�� �ַ� �޸𸮸� �̸� �Ҵ��ϰ��� �� ��, 
		/// Ư�� �޸� ��ġ�� ��ü�� ���� �����ϰ��� �� �� ���˴ϴ�. 
		/// ���� ���, �޸� Ǯ�̳� �޸� �Ŵ����� ����� �� �����ϰ� Ȱ��� �� �ֽ��ϴ�.

		new(header)MemoryHeader(size); /// placement new --> header �޸� ��ġ�� MemoryHeader�� size ũ�� ��ŭ �����͸� �����.
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}
#pragma endregion
};



DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) // 16 ����Ʈ ���� SLIST �� ����ϱ� ���ؼ��� 16 ����Ʈ ������ ���־���Ѵ�. (= ��ü �Ҵ����� �� �� �ּҰ� 16���� ������� ��)
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

	void		  Push(MemoryHeader* ptr);	/// �޸𸮸� �ݳ��ϰڴ�. 
	MemoryHeader* Pop();					/// �޸𸮸� �޾ƿͼ� ����ϰڴ�.


};

