#pragma once

/// +------------------------
///		  NET ADDRESS
/// ------------------------+

class NetAddress
{
private:
	SOCKADDR_IN		mSockAddr = {};

public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockAddr);
	NetAddress(std::wstring ip, uint16 port);

	SOCKADDR_IN&	GetSockAddr() { return mSockAddr; }
	std::wstring	GetIpAddress();
	uint16			GetPort() { return ::ntohs(mSockAddr.sin_port); }

public:
#pragma region HELPER FUNC
	static IN_ADDR	Ip2Address(const WCHAR* ip);

#pragma endregion

};

