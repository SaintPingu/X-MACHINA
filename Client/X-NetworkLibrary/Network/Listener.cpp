#include "pch.h"
#include "Listener.h"
#include "SocketData.h"
#include "NetworkInterface.h"
#include "Session.h"
#include "NetworkManager.h"




Listener::Listener()
	: NetworkObject()
{
	NetworkObject::SetName("Listener");
}

Listener::~Listener()
{
	Close();
	for (auto obj : mAccepts) {
		obj->DecRef_Session();
		SAFE_DELETE(obj);
	}
	mAccepts.clear();
	mOwnerNI = nullptr;

}


void Listener::Register_OverlappedIO_Accept(Overlapped_Accept* overlapped)
{
	SPtr_Session session = mOwnerNI->CreateSession(); // Register IOCP 

	overlapped->Clear_OVERLAPPED();
	overlapped->SetSession(session);

	DWORD recvBytes     = 0;

	SOCKET ListenSock  = GetSocketData().GetSocket();
	SOCKET ClientSock  = session->GetSocketData().GetSocket();
	BYTE*  RecvBufPtr  = session->GetRecvPktBuf().GetWritePos();
	bool result        = NETWORK_MGR->AcceptEx()(ListenSock
											  , ClientSock 
											  , RecvBufPtr 
											  , 0
											  , sizeof(SOCKADDR_IN) + 16
											  , sizeof(SOCKADDR_IN) + 16
											  , OUT &recvBytes
											  , static_cast<LPOVERLAPPED>(overlapped));
											  
	if (FALSE == result) {
		const INT32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING) {
			// Re Register Overlapped IO Accept 
			Register_OverlappedIO_Accept(overlapped);
		}
	}
	
}

void Listener::Process_OverlappedIO_Accept(Overlapped_Accept* overlapped)
{
	SPtr_Session session = overlapped->GetSession();

	if (false == session->GetSocketData().SetUpdateAcceptSocket(GetSocketData().GetSocket())) {
		Register_OverlappedIO_Accept(overlapped);
		return;
	}

	SOCKADDR_IN sockAddress;
	INT32 sizeofSockAddr = sizeof(sockAddress);
	if(SOCKET_ERROR == ::getpeername(session->GetSocketData().GetSocket()
									, OUT reinterpret_cast<SOCKADDR*>(&sockAddress)
									, &sizeofSockAddr))
	{
		Register_OverlappedIO_Accept(overlapped);
		return;
	}

	/// +--------------------
	///   Client Connected
	/// --------------------+
	session->GetSocketData().SetSockAddrIn(sockAddress);
	session->ProcessIO(OverlappedIO::Type::Connect);
	
	Register_OverlappedIO_Accept(overlapped); // ´Ù½Ã Accept

}

void Listener::Dispatch(OverlappedObject* overlapped, UINT32 bytes)
{
	Overlapped_Accept* accept = reinterpret_cast<Overlapped_Accept*>(overlapped);
	Process_OverlappedIO_Accept(accept);
}


bool Listener::Start(std::wstring ip, UINT16 portNum, SPtr_NI netInterface)
{
	mOwnerNI = netInterface;
	/* Create Listen Socket + Update Socket Option */
	SocketData ListenSockData = {};
	ListenSockData.CreateSocket();
	ListenSockData.Init(ip, portNum);
	
	if (ListenSockData.SetReuseAddress(true) == false)				
		return false;
	if (ListenSockData.SetLinger(0, 0) == false)						
		return false;
	if (ListenSockData.Bind(ListenSockData.GetSockAddr()) == false)	
		return false;
	if (ListenSockData.Listen() == false)							
		return false;
	NetworkObject::SetSocketData(ListenSockData);

	/* Register Iocp */
	if (mOwnerNI->RegisterIocp(shared_from_this()) == false)
		return false;

	return true;
}

void Listener::RegisterAccept()
{
	/* Register Accept IO */
	const UINT32 MaxAcceptCnt = mOwnerNI->GetMaxSessionCnt();
	for (UINT32 i = 0; i < MaxAcceptCnt; ++i) {

		Overlapped_Accept* acceptIO = new Overlapped_Accept();
		acceptIO->SetOwner(shared_from_this());
		mAccepts.push_back(acceptIO);
		Register_OverlappedIO_Accept(acceptIO);

	}
}

void Listener::DecRef_Accepts()
{
	for (auto& acc : mAccepts) {
		acc->DecRef_NetObj();
		acc->DecRef_Session();
	}


}

void Listener::Close()
{
	SocketData ListenSockData = NetworkObject::GetSocketData();
	ListenSockData.Close();

}


