#pragma once


/// ====================================
///						   mCursor
///							��
/// [][][][][]...[][][][][][] (mSize)
/// ====================================

class BufferWriter
{
private:
	BYTE*			mBuffer    = nullptr;
	uint32			mSize      = 0;
	uint32			mCursor    = 0;

public:
	BufferWriter();
	BufferWriter(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferWriter();

	BYTE*			GetBuffer()			{ return mBuffer; }
	uint32			GetSize()			{ return mSize; }			// ���� �� ũ��
	uint32			GetWriteSize()		{ return mCursor; }			// ���� ��� ������ 
	uint32			GetFreeSize()		{ return mSize - mCursor; }	// ���� ���� ũ�� 

	template<typename T>
	bool			Write(T* src) { return Write(src, sizeof(T)); }
	bool			Write(void* src, uint32 len);

	template<typename T>
	T*				Reserve(uint16 count = 1);

	template<typename T>
	BufferWriter&	operator<<(T&& src); // rvalue ���� : std::move �� ���� ������ �̵� (���ʿ��� ���縦 ���´�.)


};

template<typename T>
T* BufferWriter::Reserve(uint16 count)
{
	if (GetFreeSize() < (sizeof(T) * count))
		return nullptr;

	T* ret = reinterpret_cast<T*>(&mBuffer[mCursor]);
	mCursor += (sizeof(T) * count);
	return ret;
}

template<typename T>
BufferWriter& BufferWriter::operator<<(T&& src)
{
	using DataType = std::remove_reference_t<T>;
	*reinterpret_cast<DataType*>(&mBuffer[mCursor]) = std::forward<DataType>(src);
	mCursor += sizeof(T);
	return *this;
}

/// =============================================================
/// C2440 '=': unsigned int ���� uint32 & (��)�� ��ȯ�� �� �����ϴ�. 
/// =============================================================
/// > �ذ��
//  *reinterpret_cast<T*>(&mBuffer[mCursor]) = std::move(src);
/// > --->
//  *reinterpret_cast<std::remove_reference_t<T>*>(&mBuffer[mCursor]) = std::forward(src);
/// > 1. std::remove_reference_t<T>		: rvalue ����
/// > 2. std::forward(src)				: universal ���� 
/// =============================================================
///	T&& src �� 1000 uint64 ���� �������� 
///	const uint64&�� �Ǿ� ���´�. 
///
///	�׷��� reinterpret_cast<const uint64&*> �� ġȯ�Ǹ鼭 ������ �߻��Ѵ�. 
///	�̷��� ���۷��� �������� �����͸� �����ؾ��ϴ� ���� ������ �߻��� ��

/// lvalue 
/// rvalue
/// universal = rvalue or lvalue 
/// 
///  int x = 42;
/// std::move�� ����Ͽ� lvalue�� rvalue�� ĳ����
///int&& movedValue = std::move(x);
/// std::move �� ���� ������ �������� movedvalue �� �Ѿ
/// ���� x �� ���ǵ��� ���� ���� �ȴ�. 
/// x�� �����ϴ� ���� �ǰ����� �����ϸ� �ȵȴ�. �����Ϸ��� ��� ����,,, 