#include "pch.h"
#include "SocketData.h"
#include "NetworkManager.h"


SocketData::SocketData()
{
}

SocketData::~SocketData()
{
}

bool SocketData::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT & bytes, NULL, NULL);
}


void SocketData::Init(std::wstring ip, UINT16 port)
{
	mIP   = ip;
	mPort = port;

	/* Socket Address */
	::memset(&mSockAddr, 0, sizeof(mSockAddr));

	mSockAddr.sin_family	= AF_INET;
	mSockAddr.sin_addr		= NETWORK_MGR->Ip2Address(ip.c_str());
	mSockAddr.sin_port		= ::htons(port); // 포트 번호 : 16비트 숫자-host to Network - short(16bit)

	/* Socket */
	CreateSocket();
	int NalgeOff_Flag = 1; // Nagle Off;
	SetNagleAlgorithmOpt(NalgeOff_Flag);

}

SOCKET SocketData::CreateSocket()
{
	mSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	return mSocket;
}

void SocketData::Clear()
{
}

bool SocketData::Bind(SOCKADDR_IN netAddr)
{
	return SOCKET_ERROR != ::bind(mSocket, reinterpret_cast<const SOCKADDR*>(&netAddr), sizeof(SOCKADDR_IN));
}

bool SocketData::BindAnyAddress(UINT16 port)
{
	SOCKADDR_IN Address{};

	Address.sin_family      = AF_INET;
	Address.sin_addr.s_addr = ::htonl(INADDR_ANY);
	Address.sin_port        = ::htons(port);

	return SOCKET_ERROR != ::bind(mSocket, reinterpret_cast<const SOCKADDR*>(&Address), sizeof(Address));
}

bool SocketData::BindWindowsFunction(GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(mSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT & bytes, NULL, NULL);
}

bool SocketData::Listen(INT32 backlog)
{
	return SOCKET_ERROR != ::listen(mSocket, backlog);
}

void SocketData::Close()
{
	if (mSocket != INVALID_SOCKET)
		::closesocket(mSocket);
	mSocket = INVALID_SOCKET;
}

bool SocketData::SetLinger(UINT16 onoff, UINT16 linger)
{
	LINGER option;
	option.l_onoff = onoff;
	option.l_linger = linger;
	return SetSockOpt<LINGER>(mSocket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketData::SetReuseAddress(bool flag)
{
	return SetSockOpt<bool>(mSocket, SOL_SOCKET, SO_REUSEADDR, flag);
}





bool SocketData::SetUpdateAcceptSocket(SOCKET listenSocket)
{
	return SetSockOpt(mSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

bool SocketData::SetNagleAlgorithmOpt(int TCP_NoDelay)
{
	::setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&TCP_NoDelay, sizeof(int));
	return false;
}

std::wstring SocketData::GetIpAddress()
{
	WCHAR buffer[100]{};
	size_t stringBufSize = static_cast<size_t>(sizeof(buffer) / sizeof(buffer[0]));
	::InetNtopW(AF_INET, &mSockAddr.sin_addr, buffer, /*개수*/ stringBufSize); // InetNtoP : Network to Pointer(문자열) 
	return std::wstring(buffer);
}
