#pragma once

/// +-------------------------------------------------
///				 Client Network Manager 
/// __________________________________________________
///		         클라이언트 네트워크 매니저
/// 
/// 기능 : 서버와의 통신 및 수신받은 패킷을 처리하는 역할 
///        Lock은 최대한 여기서만 걸 것을 지향한다.  
/// (Lock이 중구난방 있으면 나중에 버그원인을 찾기 굉장히 힘들다.)
/// 
/// [ Front Events Queue ] -------> Process Events  ( GameLoop(main) Thread )
///		↑
///    LOCK -- ( Change )
///		↓
/// [ Back  Events Queue ] <------  Register Events ( Worker(Server) Threads)
/// -------------------------------------------------+

#include "../include/NetworkEvents.h"
#include "Scene.h"

#define NETWORK_MGR ClientNetworkManager::GetInst()
struct NetSceneEventQueue 
{
	Concurrency::concurrent_queue<sptr<NetworkEvent::Scene::EventData>> EventsQueue{};
};

class ClientNetworkManager
{
	DECLARE_SINGLETON(ClientNetworkManager);

private:
	USE_LOCK;
	Concurrency::concurrent_unordered_map<UINT32, sptr<GridObject>> mRemotePlayers{}; /* sessionID, otherPlayer */
	NetSceneEventQueue	mSceneEvnetQueue[2];		// FRONT <-> BACK 
	std::atomic_int	    mFrontSceneEventIndex = 0;	// FRONT SCENE EVENT QUEUE INDEX 
	std::atomic_int	    mBackSceneEventIndex = 1;	// BACK SCENE EVENT QUEUE INDEX 

public:
	ClientNetworkManager();
	~ClientNetworkManager();


public:
	void ProcessEvents();
	void SwapEventsQueue(); // USE LOCK! 
	void RegisterEvent(sptr<NetworkEvent::Scene::EventData> data);

};

