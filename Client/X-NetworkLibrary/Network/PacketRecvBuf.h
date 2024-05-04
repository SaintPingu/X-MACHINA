#pragma once


/// +-----------------------------------------------
///				      PacketRecvBuf 
/// ________________________________________________

/// -----------------------------------------------+

class PacketRecvBuf
{
public:
	enum class Info {
		bufferCount = 10,
		Size        = 0x10'000 // 64kb
	};

private:
	std::vector<BYTE> mBuffer = {};

	UINT32 mCapacity          = 0;
	UINT32 mBufferSize        = 0;
	UINT32 mRead_Idx          = 0;
	UINT32 mWrite_Idx         = 0;

	void* mReturnBlockPtr = nullptr;
public:
	PacketRecvBuf() = default;
	PacketRecvBuf(UINT32 bufSize);
	~PacketRecvBuf();

public:
	void Clean();
	bool OnRead(UINT32 numOfBytes);
	bool OnWrite(UINT32 numOfBytes);

	BYTE*	GetReadPos()  { return &mBuffer[mRead_Idx]; }
	BYTE*	GetWritePos() { return &mBuffer[mWrite_Idx]; }
	UINT32	GetDataSize() { return mWrite_Idx - mRead_Idx; }
	UINT32	GetFreeSize() { return mCapacity - mWrite_Idx; }

};

