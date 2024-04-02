#pragma once

/*---------------
   RefCountable
----------------*/

class RefCountable
{

protected:
	std::atomic<int32> mRefCount;

public:
	RefCountable() : mRefCount(1) { }
	virtual ~RefCountable() { }

	int32 GetRefCount() { return mRefCount; }

	int32 AddRef() { return ++mRefCount; }
	int32 ReleaseRef()
	{
		int32 refCount = --mRefCount;
		if (refCount == 0)
		{
			delete this;
		}
		return refCount;
	}

};

/*---------------
   SharedPtr
----------------*/

template<typename T>
class TSharedPtr
{
private:
	T* mPtr = nullptr;

public:
	TSharedPtr() { }
	TSharedPtr(T* ptr) { Set(ptr); }

	// ����
	TSharedPtr(const TSharedPtr& rhs) { Set(rhs.mPtr); }
	// �̵�
	TSharedPtr(TSharedPtr&& rhs) { mPtr = rhs.mPtr; rhs.mPtr = nullptr; }
	// ��� ���� ����
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs) { Set(static_cast<T*>(rhs.mPtr)); }

	~TSharedPtr() { Release(); }

public:
	// ���� ������
	TSharedPtr& operator=(const TSharedPtr& rhs)
	{
		if (mPtr != rhs.mPtr)
		{
			Release();
			Set(rhs.mPtr);
		}
		return *this;
	}

	// �̵� ������
	TSharedPtr& operator=(TSharedPtr&& rhs)
	{
		Release();
		mPtr = rhs.mPtr;
		rhs.mPtr = nullptr;
		return *this;
	}

	bool		operator==(const TSharedPtr& rhs) const { return mPtr == rhs.mPtr; }
	bool		operator==(T* ptr) const { return mPtr == ptr; }
	bool		operator!=(const TSharedPtr& rhs) const { return mPtr != rhs.mPtr; }
	bool		operator!=(T* ptr) const { return mPtr != ptr; }
	bool		operator<(const TSharedPtr& rhs) const { return mPtr < rhs.mPtr; }
	T*			operator*() { return mPtr; }
	const T*	operator*() const { return mPtr; }
				operator T* () const { return mPtr; }
	T*			operator->() { return mPtr; }
	const T*	operator->() const { return mPtr; }

	bool IsNull() { return mPtr == nullptr; }

private:
	inline void Set(T* ptr)
	{
		mPtr = ptr;
		if (ptr)
			ptr->AddRef();
	}

	inline void Release()
	{
		if (mPtr != nullptr)
		{
			mPtr->ReleaseRef();
			mPtr = nullptr;
		}
	}


};