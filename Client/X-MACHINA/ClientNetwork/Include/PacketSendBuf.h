#pragma once


/// +-----------------------------------------------
///					 PacketSendBuf 
/// ________________________________________________
/// 
/// ※ 주의사항
///  	
/// - WSASend()를 한 후 완료 통지가 올때까지 데이터가 
///   유지되어야한다. ==>  std::shared_ptr 로 관리하자!
/// -----------------------------------------------+

/*
	어차피 패킷은 프로토콜 아이디에 따라 패킷의 크기는 정해져있다.
	ex) Transform 패킷 이라거 할 때 sizeof(float) * 3 * 3 이겠지?

	그래서 메모리 풀을 만들어 놓자. 
	각 패킷에 해당하는 메모리 풀을 미리 여러개 만들어 놓고 
	패킷을 보낼 때 메모리 풀에서 메모리를 빼오고 
	Make_Shared 로 만든 다음에 갖다 썼다가
	다시 메모리 풀에 갖다가 넣자.

	각 패킷에 맞는 메모리를 메모리 풀에서 긁어 올때는 
	unordered_map 을 이용하자
	O(1) 

	그렇다면 가변 길이 버퍼는 어떻게 하나?
	그래서 32 64 128 256 512 바이트 메모리풀들을 미리 만들어놓고
	여기다가 쓰자 !
*/


class PacketSendBuf
{
public:
	/* 다시 반납하기 위해서 필요한 정보 */
	BYTE*  mPtrFromMemPool  = nullptr;
	UINT16 mMemoryPoolSize  = 0;

private:
	BYTE*					mBuffer          = {}; // 메모리 포인터
	UINT32					mBufferTotalSize = 0;  // SListMemoryPool 에서 가져온 메모리의 전체 크기 

public:
	PacketSendBuf(BYTE* ptrMem, UINT16 MemSize, BYTE* buffer, UINT32 allocSize);
	~PacketSendBuf();

	void    SetOwnerInfo(BYTE* ownerPtr, UINT16 ownerMemSize) { mPtrFromMemPool = ownerPtr; mMemoryPoolSize = ownerMemSize; }
	void    SetPacketHederInfo(class PacketHeader info);

	void     SetBuffer(BYTE* buf);


	BYTE*	GetBuffer()	   { return mBuffer; }
	UINT32	GetTotalSize() { return mBufferTotalSize; }
};

