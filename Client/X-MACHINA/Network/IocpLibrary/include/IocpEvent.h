#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	//PreRecv,
	Recv,
	Send
};

/// +------------------------
///		*  IOCP EVENT  *
/// ------------------------+

class IocpEvent : public OVERLAPPED
{

public:
	EventType		eventType;
	IocpObjectRef	owner;

public:
	IocpEvent(EventType type);

	void			Init();


};

/// +------------------------
///		  CONNECT EVENT
/// ------------------------+
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) { }
};

/// +------------------------
///		  DISCONNECT EVENT
/// ------------------------+
class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) { }
};


/// +------------------------
///		  ACCEPT EVENT
/// ------------------------+
class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) { }

public:
	SessionRef	session = nullptr;
};

/// +------------------------
///		  RECV EVENT
/// ------------------------+
class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) { }
};

/// +------------------------
///		  SEND EVENT
/// ------------------------+
class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) { }
	 
	NetVector<SPtr_SendPktBuf> sendBuffers;
};