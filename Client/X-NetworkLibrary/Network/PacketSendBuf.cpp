#include "pch.h"
#include "PacketSendBuf.h"
#include "SListMemoryPool.h"
#include "SendBuffersFactory.h"
#include "ThreadManager.h"


PacketSendBuf::PacketSendBuf(BYTE* ptrMem, UINT16 MemSize, BYTE* buffer, UINT32 allocSize)
	: mPtrFromMemPool(ptrMem)
	, mMemoryPoolSize(MemSize)
	, mBuffer(buffer)
	, mBufferTotalSize(allocSize)
{
}

PacketSendBuf::~PacketSendBuf()
{
	/* �޸� �ݳ� */
	void* MemPtr = static_cast<BYTE*>(mBuffer) - ((mMemoryPoolSize - mBufferTotalSize));
	SENDBUF_FACTORY->Push_VarPkt(mMemoryPoolSize, MemPtr); /* �޸� Ǯ�� �ݳ�! */
	SENDBUF_FACTORY->Push_SendPkt(this);
}



void PacketSendBuf::SetPacketHederInfo(PacketHeader info)
{
	
}

void PacketSendBuf::SetBuffer(BYTE* buf)
{
	mBuffer = buf;
}
