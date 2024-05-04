#pragma once

/// +-----------------------------------------------
///				   OverlappedObject 
/// ________________________________________________
/// Iocp �Լ��� ����� Overlapped Object �̴�.
/// 
/// �ش� ������Ʈ�� ::GetQueuedCompletionStatus�� ���ؼ�
/// ���� �� IoType�� ���ؼ� � Ÿ������ �� �� �ִ�. 
/// �ش� IoType �� ���� Overlapped_(Accpet, Connect,,)�� 
/// ����ȯ�Ͽ� �ʿ��� �����͸� �����
/// ________________________________________________
/// ���� 
///					OVERLAPPED 
///						��
///				 OverlappedObject
///						��
///				 Overlapped_Accept
///				 Overlapped_Connect
///				 Overlapped_DisConnect
///				 Overlapped_Send
///				 Overlapped_Recv
/// 
/// ----------------------------------------------+

namespace OverlappedIO {
	enum class Type : UINT8 {
		None,
		Accept,
		Connect,
		DisConnect,
		Send,
		Recv,
	};
};

class OverlappedObject : public WSAOVERLAPPED
{
private:
	UINT8				mIoType = static_cast<UINT8>(OverlappedIO::Type::None);
	SPtr_NetObj			mOwner  = nullptr;		// Session? Listener?

public:
	OverlappedObject(OverlappedIO::Type ioType);
	 ~OverlappedObject();

public:
	/* Get */
	SPtr_NetObj			GetOwner()	{ return mOwner; }
	OverlappedIO::Type	GetIoType() { return static_cast<OverlappedIO::Type>(mIoType); }

	/* Set */
	void				SetOwner(SPtr_NetObj owner) { mOwner = owner; }

	/* Ref Counting �� */
	void DecRef_NetObj() { mOwner = nullptr; }
	void Clear_OVERLAPPED();
};

/* Accept Overlapped Obeject */
class Overlapped_Accept : public OverlappedObject 
{
private:
	SPtr_Session  mSession = nullptr;

public:
	SPtr_Session GetSession() { return mSession; }
	void SetSession(SPtr_Session session) { mSession = session; }
	void DecRef_Session() { mSession = nullptr; }
public:
	Overlapped_Accept() 
		: OverlappedObject(OverlappedIO::Type::Accept)
	{
	}
	~Overlapped_Accept()
	{
		mSession = nullptr;
	}
};

/* Connect Overlapped Object */
class Overlapped_Connect : public OverlappedObject
{
public:
	Overlapped_Connect() 
		: OverlappedObject(OverlappedIO::Type::Connect)
	{
	}
	~Overlapped_Connect()
	{
	}
};

/* DisConnect Overlapped Object */
class Overlapped_DisConnect : public OverlappedObject
{
public:
	Overlapped_DisConnect() 
		: OverlappedObject(OverlappedIO::Type::DisConnect)
	{
	}
	 ~Overlapped_DisConnect()
	{
	}
};

/* Send Overlapped Object */
class Overlapped_Send : public OverlappedObject
{
private:
	std::vector<SPtr_SendPktBuf> mSendBuffers;

public:
	size_t BufSize()								{ return mSendBuffers.size(); }
	void   BufPush(SPtr_SendPktBuf sendBuf)			{ mSendBuffers.emplace_back(sendBuf); }
	void   BufClear()								{ mSendBuffers.clear(); }
	std::vector<SPtr_SendPktBuf>& GetSendBuffers()	{ return mSendBuffers; }

	void ReleaseSendBuffersReferenceCount() { mSendBuffers.clear(); }

public:
	Overlapped_Send() 
		: OverlappedObject(OverlappedIO::Type::Send)
	{
	}
	 ~Overlapped_Send()
	{
	}
};

/* Recv Overlapped Object */
class Overlapped_Recv : public OverlappedObject
{
public:
	Overlapped_Recv() 
		: OverlappedObject(OverlappedIO::Type::Recv)
	{
	}
	 ~Overlapped_Recv()
	{
	}
};