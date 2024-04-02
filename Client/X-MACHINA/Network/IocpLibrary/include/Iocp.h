#pragma once

/// +-------------------
///   IOCP 등록 오브젝트
/// -------------------+

class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract; 	// Iocp Type 에 따라 이벤트 처리한다. 

};

/*--------------
	Iocp
---------------*/

class Iocp
{
private:
	HANDLE		mIocpHandle{};

public:
	Iocp();
	~Iocp();

	HANDLE		GetHandle() { return mIocpHandle; }

	// 등록할 IocpObject
	bool		Register(IocpObjectRef iocpObject); 
	// worker Thread 들이 Dispatch 함수를 실행해서 일감이 있는지 확인한다
	bool		Dispatch(uint32 timeoutMs = INFINITE);


};