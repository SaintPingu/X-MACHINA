#pragma once

/// ====================================
///						   mCursor
///							��
/// [][][][][]...[][][][][][] (mSize)
/// ====================================
class BufferReader
{

private:
	BYTE*			mBuffer   = nullptr;
	uint32			mSize     = 0;
	uint32			mCursor   = 0; // pos

public:
	BufferReader();
	BufferReader(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferReader();

	BYTE*			GetBuffer() { return mBuffer; }
	uint32			Size() { return mSize; }
	uint32			ReadSize() { return mCursor; }
	uint32			GetFreeSize() { return mSize - mCursor; }

	/// ������ Ȯ�ο�
	template<typename T>
	bool			Peek(T* dest) { return Peek(dest, sizeof(T)); }
	bool			Peek(void* dest, uint32 len);

	/// ������ �б�
	template<typename T>
	bool			Read(T* dest) { return Read(dest, sizeof(T)); }
	bool			Read(void* dest, uint32 len);

	template<typename T>
	BufferReader&	operator>>(OUT T& dest);

};

template<typename T>
inline BufferReader& BufferReader::operator>>(OUT T& dest)
{
	dest = *reinterpret_cast<T*>(&mBuffer[mCursor]);
	mCursor += sizeof(T); // ���� ��ŭ Ŀ���� �̵�
	return *this;
}