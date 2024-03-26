#pragma once


/// ====================================
///						   mCursor
///							↓
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
	uint32			GetSize()			{ return mSize; }			// 버퍼 총 크기
	uint32			GetWriteSize()		{ return mCursor; }			// 버퍼 사용 사이즈 
	uint32			GetFreeSize()		{ return mSize - mCursor; }	// 버퍼 여분 크기 

	template<typename T>
	bool			Write(T* src) { return Write(src, sizeof(T)); }
	bool			Write(void* src, uint32 len);

	template<typename T>
	T*				Reserve(uint16 count = 1);

	template<typename T>
	BufferWriter&	operator<<(T&& src); // rvalue 참조 : std::move 와 같은 데이터 이동 (불필요한 복사를 막는다.)


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
/// C2440 '=': unsigned int 에서 uint32 & (으)로 변환할 수 없습니다. 
/// =============================================================
/// > 해결법
//  *reinterpret_cast<T*>(&mBuffer[mCursor]) = std::move(src);
/// > --->
//  *reinterpret_cast<std::remove_reference_t<T>*>(&mBuffer[mCursor]) = std::forward(src);
/// > 1. std::remove_reference_t<T>		: rvalue 참조
/// > 2. std::forward(src)				: universal 참조 
/// =============================================================
///	T&& src 에 1000 uint64 값이 들어왔으면 
///	const uint64&가 되어 들어온다. 
///
///	그러면 reinterpret_cast<const uint64&*> 로 치환되면서 문제가 발생한다. 
///	이러면 레퍼런스 참조값의 포인터를 추출해야하다 보니 문제가 발생한 것

/// lvalue 
/// rvalue
/// universal = rvalue or lvalue 
/// 
///  int x = 42;
/// std::move를 사용하여 lvalue를 rvalue로 캐스팅
///int&& movedValue = std::move(x);
/// std::move 를 통해 데이터 소유권이 movedvalue 로 넘어감
/// 따라서 x 는 정의되지 않은 값이 된다. 
/// x에 접근하는 것은 되겠지만 접근하면 안된다. 컴파일러가 경고도 안함,,, 