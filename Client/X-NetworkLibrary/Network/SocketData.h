#pragma once

/// +-----------------------------------------------
///					Socket Data 
/// ________________________________________________
///			 SOCKET, SOCKADDR_IN Wrapper 클래스
/// 소켓과 소켓 주소 정보를 다루는 클래스 
/// 소켓 옵션을 추가한다.  
/// -----------------------------------------------+

class SocketData
{
private:
	SOCKET				mSocket		= INVALID_SOCKET;
	SOCKADDR_IN			mSockAddr	= {};

	std::wstring		mIP			= {};
	UINT16				mPort		= {};
public:
	SocketData();
	~SocketData();

	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);


public:
	void	Init(std::wstring ip, UINT16 port);
	SOCKET	CreateSocket();
	void	Clear();
		
	const SOCKET& GetSocket() { return mSocket; }
	void SetSockAddrIn(SOCKADDR_IN& sockaddrIn) { mSockAddr = sockaddrIn; }
public:
	/// +-----------------------------
	///		Bind / Listen / close 
	/// -----------------------------+
	bool Bind(SOCKADDR_IN netAddr);
	bool BindAnyAddress(UINT16 port);
	bool BindWindowsFunction(GUID guid, LPVOID* fn);
	bool Listen(INT32 backlog = SOMAXCONN);
	void Close();

	/// +-----------------------------
	///		    SOCKET OPTION 
	/// -----------------------------+
	bool SetLinger(UINT16 onoff, UINT16 linger);
	bool SetReuseAddress(bool flag);
	bool SetUpdateAcceptSocket(SOCKET listenSocket);

	/// +------------------------
	///		   NET ADDRESS
	/// ------------------------+
	SOCKADDR_IN& 	GetSockAddr() { return mSockAddr; }
	std::wstring	GetIpAddress();
	UINT16			GetPort() { return ::ntohs(mSockAddr.sin_port); }

};

template<typename T>
static inline bool SetSockOpt(SOCKET socket, INT32 level, INT32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}