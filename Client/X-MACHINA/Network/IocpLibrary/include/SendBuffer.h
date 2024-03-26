#pragma once

class SendBufferChunk;

///+---------------------------
///			SEND BUFFER 
/// --------------------------+

class SendBuffer
{
private:
	BYTE*				mBuffer;
	uint32				mAllocSize = 0;
	uint32				mWriteSize = 0;
	SendBufferChunkRef	mOwner;

public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize);
	~SendBuffer();

	BYTE*		GetBuffer() { return mBuffer; }
	uint32		GetAllocSize() { return mAllocSize; }
	uint32		GetWriteSize() { return mWriteSize; }
	void		Close(uint32 writeSize);


};

///+---------------------------
///		 SEND BUFFER CHUNK
/// --------------------------+
// 큰 메모리를 미리 할당받고 그 메모리를 쪼개서 SendBuffer를 사용할 용도
// [[		][	 ][			][ ]...		]
//		↓	   ↓	   ↓	  ↓
//	  SendBuffer할당..

enum class SendBufferChunkInfo
{
	Size = 6000,
	_CountOf,
};
class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk>
{
	enum
	{
		SEND_BUFFER_CHUNK_SIZE = 6000
	};


private:
	NetArray<BYTE, SEND_BUFFER_CHUNK_SIZE>		mBuffer   = {};
	bool									mIsOpen   = false;
	uint32									mUsedSize = 0;

public:
	SendBufferChunk();
	~SendBufferChunk();

	void				Reset();
	SPtr_SendPktBuf		Open(uint32 allocSize);
	void				Close(uint32 writeSize);

	bool				IsOpen() { return mIsOpen; }
	BYTE*				GetBuffer() { return &mBuffer[mUsedSize]; }
	uint32				GetFreeSize() { return static_cast<uint32>(mBuffer.size()) - mUsedSize; }

};

///+---------------------------
///	    SEND BUFFER MANAGER
/// --------------------------+ 
#define SENDPACKET_MGR SendBufferManager::GetInst()

class SendBufferManager
{
	DECLARE_SINGLETON(SendBufferManager);

private:
	USE_LOCK;
	NetVector<SendBufferChunkRef> _sendBufferChunks;

public:
	SendBufferManager();
	~SendBufferManager();

public:
	SPtr_SendPktBuf		Open(uint32 size);

private:
	SendBufferChunkRef	Pop();
	void				Push(SendBufferChunkRef buffer);

	static void			PushGlobal(SendBufferChunk* buffer);


};
