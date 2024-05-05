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

#include "ClientNetwork/Contents/NetworkEvents.h"
#include "Scene.h"
#include "InputMgr.h"


#define NETWORK_MGR ClientNetworkManager::GetInst()
struct NetSceneEventQueue 
{
	Concurrency::concurrent_queue<sptr<NetworkEvent::Scene::EventData>> EventsQueue{};
};


class ClientNetworkManager
{
	DECLARE_SINGLETON(ClientNetworkManager);

private:
	Lock::SRWLockGuard mSRWLock{};
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
	void RegisterEvent(sptr<NetworkEvent::Scene::EventData> data);
	
	long long GetTimeStamp();

public:
	/* Send Client Packet */
	void Send_CPkt_KeyInput(
					GameKeyInfo::KEY		 key
				,	GameKeyInfo::KEY_STATE	 KeyState
				,	GameKeyInfo::MoveKey	 moveKey
				,	Vec2					 mouseDelta);

	void Send_CPkt_Transform(Vec3 Pos, Vec3 Rot, Vec3 Scale, Vec3 SpineLookDir, long long timestamp);


};

