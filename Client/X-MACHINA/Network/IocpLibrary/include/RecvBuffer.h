#pragma once

/// +------------------------------		
///			  RECV BUFFER 
///	------------------------------+
/// 
/// (Read Cursor)
/// r -> 한바이트씩 읽는다.
/// [][][][][][][][][][][][][][][]
/// w -> 한바이트씩 쓴다. ( w 커서 위치를 기준으로 계속 써내려간다. )
/// (WriteCursor)
/// 
/// 1. r위치와 w위치가 같으면 쓴것을 다 읽은 것으로 rw 를 처음위치로 옮긴다.
/// 2. [][][][][r][+][+][w][끝] <- 이러한 상황에서
///    더이상 write 할 수 없으므로 [r][+][+][w][][][][]... r 과 w를 앞으로 옮긴다. 
/// 
/// -------------------------------


enum class RecvBufferInfo
{
	BufferCount = 10,
	_CountOf,

};
class RecvBuffer
{

private:
	NetVector<BYTE>	mBuffer{};

	int32			mCapacity   = 0;
	int32			mBufferSize = 0;
	int32			mReadPos    = 0;
	int32			mWritePos   = 0;

public:
	RecvBuffer() = default;
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();

	void			Clean();
	bool			OnRead(int32 numOfBytes);
	bool			OnWrite(int32 numOfBytes);

	BYTE*			GetReadPos()	{ return &mBuffer[mReadPos]; }
	BYTE*			GetWritePos()	{ return &mBuffer[mWritePos]; }
	int32			GetDataSize()	{ return mWritePos - mReadPos; }
	int32			GetFreeSize()	{ return mCapacity - mWritePos; }


};

