#include "pch.h"
#include "PacketRecvBuf.h"
#include "MemoryManager.h"
#include "SListMemoryPool.h"



PacketRecvBuf::PacketRecvBuf(UINT32 bufSize)
	: mBufferSize(bufSize)
{
	mCapacity = bufSize * static_cast<UINT32>(PacketRecvBuf::Info::bufferCount);

	/* Add SListMemoryPool */
	const size_t MemoryBlockSize	 = mCapacity;
	const size_t NumBlock			 = 1;
	MEMORY->AddSListMemoryPool("RecvBuf", MemoryBlockSize);


	/* Use RecvBuf MemoryBlock From SListMemoryPool */
	mBuffer.reserve(MemoryBlockSize);
	//void* blockPtr = MEMORY->Allocate(MemoryBlockSize);
	void* blockPtr = MEMORY->Allocate("RecvBuf");
	mReturnBlockPtr = blockPtr;
	if (blockPtr) {
		// 메모리 블록을 0으로 초기화
		ZeroMemory(blockPtr, MemoryBlockSize);
		BYTE* blockBytes = static_cast<BYTE*>(blockPtr);
		mBuffer.insert(mBuffer.end(), blockBytes, blockBytes + MemoryBlockSize);
	}
	else {
		std::cout << "Failed To Allocate Memory Block : PacketRecvBuf\n";
	}

}

PacketRecvBuf::~PacketRecvBuf()
{
	MEMORY->Free("RecvBuf", mReturnBlockPtr);
}

void PacketRecvBuf::Clean()
{
	INT32 Datasize = GetDataSize();
	if (Datasize == 0) {
		// 딱 마침 읽기+쓰기 커서가 동일한 위치라면, 둘 다 리셋.
		mRead_Idx = mWrite_Idx = 0;
	}
	else {

		// 여유 공간이 버퍼 1개 크기 미만이면, 데이터를 앞으로 땅긴다.
		// 여유 버퍼 크기 < 단일 버퍼 크기
		if (GetFreeSize() < mBufferSize) {
			::memcpy(&mBuffer[0], &mBuffer[mRead_Idx], Datasize);
			mRead_Idx  = 0;
			mWrite_Idx = Datasize;
		}
	}
}

bool PacketRecvBuf::OnRead(UINT32 numOfBytes)
{
	if (numOfBytes > GetDataSize())
		return false;

	mRead_Idx += numOfBytes;
	return true;
}

bool PacketRecvBuf::OnWrite(UINT32 numOfBytes)
{
	if (numOfBytes > GetFreeSize())
		return false;

	mWrite_Idx += numOfBytes;
	return true;
}
