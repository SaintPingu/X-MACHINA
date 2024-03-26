#pragma once

/// +------------------------------		
///			  RECV BUFFER 
///	------------------------------+
/// 
/// (Read Cursor)
/// r -> �ѹ���Ʈ�� �д´�.
/// [][][][][][][][][][][][][][][]
/// w -> �ѹ���Ʈ�� ����. ( w Ŀ�� ��ġ�� �������� ��� �᳻������. )
/// (WriteCursor)
/// 
/// 1. r��ġ�� w��ġ�� ������ ������ �� ���� ������ rw �� ó����ġ�� �ű��.
/// 2. [][][][][r][+][+][w][��] <- �̷��� ��Ȳ����
///    ���̻� write �� �� �����Ƿ� [r][+][+][w][][][][]... r �� w�� ������ �ű��. 
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

