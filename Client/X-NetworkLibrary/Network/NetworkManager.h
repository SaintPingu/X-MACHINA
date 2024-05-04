#pragma once

/// +-----------------------------------------------
///					Network Manager 
/// ________________________________________________
///				  네트워크 기능 관리자 
/// 
/// ----------------------------------------------+

#define NETWORK_MGR NetworkManager::GetInst()
class NetworkManager
{
	DECLARE_SINGLETON(NetworkManager);

private:
	inline static LPFN_CONNECTEX		mLpfn_ConnectEx    = {};
	inline static LPFN_DISCONNECTEX		mLpfn_DisconnectEx = {};
	inline static LPFN_ACCEPTEX			mLpfn_AcceptEx     = {};

public:
	NetworkManager();
	~NetworkManager();

public:
	bool Init();
	bool WSAStartUp(INT16 major, INT16 minor);


public:
	LPFN_CONNECTEX		ConnectEx()		{ return mLpfn_ConnectEx; }
	LPFN_DISCONNECTEX	DiscconectEx()	{ return mLpfn_DisconnectEx; }
	LPFN_ACCEPTEX		AcceptEx()		{ return mLpfn_AcceptEx; }

public:
	IN_ADDR	Ip2Address(const WCHAR* ip);

};

