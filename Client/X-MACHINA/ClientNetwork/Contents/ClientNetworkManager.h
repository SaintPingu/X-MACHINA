#pragma once

/// +-------------------------------------------------
///				 Client Network Manager 
/// __________________________________________________
///		         Ŭ���̾�Ʈ ��Ʈ��ũ �Ŵ���
/// 
/// ��� : �������� ��� �� ���Ź��� ��Ŷ�� ó���ϴ� ���� 
///        Lock�� �ִ��� ���⼭�� �� ���� �����Ѵ�.  
/// (Lock�� �߱����� ������ ���߿� ���׿����� ã�� ������ �����.)
/// 
/// [ Front Events Queue ] -------> Process Events  ( GameLoop(main) Thread )
///		��
///    LOCK -- ( Change )
///		��
/// [ Back  Events Queue ] <------  Register Events ( Worker(Server) Threads)
/// -------------------------------------------------+
#undef max
#include "ClientNetwork/Contents/NetworkEvents.h"
#include "ClientNetwork/Contents/GamePlayer.h"
#include "Scene.h"
#include "InputMgr.h"


#define CLIENT_NETWORK ClientNetworkManager::GetInst()
struct NetSceneEventQueue 
{
	Concurrency::concurrent_queue<sptr<NetworkEvent::Game::EventData>> EventsQueue{};
};


class ClientNetworkManager
{
	DECLARE_SINGLETON(ClientNetworkManager);

private:
	Lock::SRWLock mSRWLock{};
	SPtr_ClientNetwork  mClientNetwork{};


	Concurrency::concurrent_unordered_map<UINT32, sptr<GridObject>> mRemotePlayers{}; /* sessionID, RemotePlayer */
	NetSceneEventQueue	mSceneEvnetQueue[2];		// FRONT <-> BACK 
	std::atomic_int	    mFrontSceneEventIndex = 0;	// FRONT SCENE EVENT QUEUE INDEX 
	std::atomic_int	    mBackSceneEventIndex = 1;	// BACK SCENE EVENT QUEUE INDEX 

public:
	ClientNetworkManager();
	~ClientNetworkManager();


public:
	void Init(std::wstring ip, UINT32 port);
	void Launch(int ThreadNum);

	void ProcessEvents();
	void SwapEventsQueue(); 
	void RegisterEvent(sptr<NetworkEvent::Game::EventData> data);
	
	long long GetTimeStamp();

public:
	/* Send Client Packet */
	void Send(SPtr_PacketSendBuf pkt);

public:
	sptr<NetworkEvent::Game::Add_RemotePlayer>		CreateEvent_Add_RemotePlayer(GamePlayerInfo info);
	sptr<NetworkEvent::Game::Remove_RemotePlayer>	CreateEvent_Remove_RemotePlayer(int32_t remID);
	sptr<NetworkEvent::Game::Move_RemotePlayer>		CreateEvent_Move_RemotePlayer(int32_t remID, Vec3 remotePos);

	long long GetCurrentTimeMilliseconds();


};

