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
#undef max
#include "ClientNetwork/Contents/NetworkEvents.h"
#include "ClientNetwork/Contents/GamePlayer.h"
#include "ClientNetwork/Contents/Script_RemotePlayer.h"
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

	int mAnimationIndex = -1;

public:
	ClientNetworkManager();
	~ClientNetworkManager();


public:
	void Init(std::wstring ip, UINT32 port);
	void Launch(int ThreadNum);

	void ProcessEvents();
	void SwapEventsQueue(); 
	void RegisterEvent(sptr<NetworkEvent::Game::EventData> data);


public:
	/* Send Client Packet */
	void Send(SPtr_PacketSendBuf pkt);

public:
	sptr<NetworkEvent::Game::Add_RemotePlayer>					CreateEvent_Add_RemotePlayer(GamePlayerInfo info);
	sptr<NetworkEvent::Game::Remove_RemotePlayer>				CreateEvent_Remove_RemotePlayer(int32_t remID);
	sptr<NetworkEvent::Game::Move_RemotePlayer>					CreateEvent_Move_RemotePlayer(int32_t remID, Vec3 remotePos, ExtData::MOVESTATE movestate);
	sptr<NetworkEvent::Game::Extrapolate_RemotePlayer>			CreateEvent_Extrapolate_RemotePlayer(int32_t remID, ExtData extdata);
	sptr<NetworkEvent::Game::ChangeAnimation_RemotePlayer>		CreateEvent_ChangeAnimation_RemotePlayer(int32_t remID, int anim_upper_idx, int anim_lower_idx, float anim_param_h, float anim_param_v);

	long long GetCurrentTimeMilliseconds();
	long long GetTimeStamp();


};

