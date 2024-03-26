#pragma once

/// +-------------------
///   IOCP ��� ������Ʈ
/// -------------------+

class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract; 	// Iocp Type �� ���� �̺�Ʈ ó���Ѵ�. 

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

	// ����� IocpObject
	bool		Register(IocpObjectRef iocpObject); 
	// worker Thread ���� Dispatch �Լ��� �����ؼ� �ϰ��� �ִ��� Ȯ���Ѵ�
	bool		Dispatch(uint32 timeoutMs = INFINITE);


};