#pragma once

/// +-------------------------------------------------
///				 Client Network Manager 
/// __________________________________________________
///		         ?´?¼?´?–¸?Š¸ ?„¤?Š¸?›Œ?¬ ë§¤ë‹ˆ???
/// 
/// ê¸°ëŠ¥ : ?„œë²„ì???˜ ?†µ?‹  ë°? ?ˆ˜?‹ ë°›ì?? ?Œ¨?‚·?„ ì²˜ë¦¬?•˜?Š” ?—­?•  
///        Lock??? ìµœë???•œ ?—¬ê¸°ì„œë§? ê±? ê²ƒì„ ì§??–¥?•œ?‹¤.  
/// (Lock?´ ì¤‘êµ¬?‚œë°? ?žˆ?œ¼ë©? ?‚˜ì¤‘ì— ë²„ê·¸?›?¸?„ ì°¾ê¸° êµ‰ìž¥?žˆ ?ž˜?“¤?‹¤.)
/// 
/// [ Front Events Queue ] -------> Process Events  ( GameLoop(main) Thread )
///		?†‘
///    LOCK -- ( Change )
///		?†“
/// [ Back  Events Queue ] <------  Register Events ( Worker(Server) Threads)
/// -------------------------------------------------+
#undef max
#include "ClientNetwork/Contents/NetworkEvents.h"
#include "ClientNetwork/Contents/GamePlayer.h"
#include "ClientNetwork/Contents/GameMonster.h"
#include "ClientNetwork/Contents/Script_NetworkRemotePlayer.h"
#include "BattleScene.h"
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
	bool mIsRunning{};
	Lock::SRWLock mSRWLock{};
	SPtr_ClientNetwork  mClientNetwork{};

	std::unordered_map<UINT32, class Script_NetworkEnemy*> mRemoteMonsters{};
	Concurrency::concurrent_unordered_map<uint32_t, GridObject*> mRemotePlayers{}; /* sessionID, RemotePlayer */
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
	void Stop();
	
	void ProcessEvents();
	void SwapEventsQueue(); 
	void RegisterEvent(sptr<NetworkEvent::Game::EventData> data);
	std::string GetLocalIPv4Address();
	GridObject* GetRemotePlayer(UINT32 id);


public:
	/* Send Client Packet */
	void Send(SPtr_PacketSendBuf pkt);

public:
	/// +---------------------------------------------------------------------------
	/// >> ?–¶?–¶?–¶?–¶?–¶ CREATE EVENT 
	/// ---------------------------------------------------------------------------+
	sptr<NetworkEvent::Game::Event_RemotePlayer::Add>					CreateEvent_Add_RemotePlayer(GamePlayerInfo info);
	sptr<NetworkEvent::Game::Event_RemotePlayer::Remove>				CreateEvent_Remove_RemotePlayer(uint32_t remID);
	sptr<NetworkEvent::Game::Event_RemotePlayer::Move>					CreateEvent_Move_RemotePlayer(uint32_t remID, Vec3 remotePos, ExtData::MOVESTATE movestate);
	sptr<NetworkEvent::Game::Event_RemotePlayer::Extrapolate>			CreateEvent_Extrapolate_RemotePlayer(uint32_t remID, ExtData extdata);
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation>		CreateEvent_UpdateAnimation_RemotePlayer(uint32_t remID, int anim_upper_idx, int anim_lower_idx, float anim_param_h, float anim_param_v);
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation>		CreateEvent_UpdateAimRotation_RemotePlayer(uint32_t remID, float aim_rotation_y);
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateWeapon>			CreateEvent_UpdateWeapon_RemotePlayer(uint32_t remID, FBProtocol::WEAPON_TYPE weaponType);
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateOnShoot>			CreateEvent_UpdateOnShoot_RemotePlayer(uint32_t remID, int bullet_id, int weapon_id, Vec3 ray);
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateOnSkill>			CreateEvent_UpdateOnSkill_RemotePlayer(uint32_t remID, FBProtocol::PLAYER_SKILL_TYPE skillType, float phero_amount);



	sptr<NetworkEvent::Game::Event_Monster::Add>						CreateEvent_Add_Monster(std::vector<GameMonsterInfo> infos);
	sptr<NetworkEvent::Game::Event_Monster::Remove>						CreateEvent_Remove_Monster(std::vector<uint32_t> Ids);
	sptr<NetworkEvent::Game::Event_Monster::Move>						CreateEvent_Move_Monster(std::vector<NetworkEvent::Game::Event_Monster::MonsterMove> infos);
	sptr<NetworkEvent::Game::Event_Monster::UpdateHP>					CreateEvent_UpdateHP_Monster(std::vector<NetworkEvent::Game::Event_Monster::MonsterHP> infos);
	sptr<NetworkEvent::Game::Event_Monster::UpdateState>				CreateEvent_UpdateState_Monster(std::vector<NetworkEvent::Game::Event_Monster::MonsterUpdateState> infos);

	sptr<NetworkEvent::Game::Event_Monster::MonsterTargetUpdate>		CreateEvent_Monster_Target(std::vector<NetworkEvent::Game::Event_Monster::MonsterTarget> infos);

	
	/// +---------------------------------------------------------------------------
	/// >> ?–¶?–¶?–¶?–¶?–¶ PROCESS EVENT 
	/// ---------------------------------------------------------------------------+
	/// REMOTE PLAYER 
	void ProcessEvent_RemotePlayer_Add(NetworkEvent::Game::Event_RemotePlayer::Add* data);
	void ProcessEvent_RemotePlayer_Remove(NetworkEvent::Game::Event_RemotePlayer::Remove* data);
	void ProcessEvent_RemotePlayer_Move(NetworkEvent::Game::Event_RemotePlayer::Move* data);
	void ProcessEvent_RemotePlayer_Extrapolate(NetworkEvent::Game::Event_RemotePlayer::Extrapolate* data);
	void ProcessEvent_RemotePlayer_UpdateAnimation(NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation* data);
	void ProcessEvent_RemotePlayer_AimRotation(NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation* data);
	void ProcessEvent_RemotePlayer_UpdateWeapon(NetworkEvent::Game::Event_RemotePlayer::UpdateWeapon* data);
	void ProcessEvent_RemotePlayer_UpdateOnShoot(NetworkEvent::Game::Event_RemotePlayer::UpdateOnShoot* data);
	void ProcessEvent_RemotePlayer_UpdateOnSkill(NetworkEvent::Game::Event_RemotePlayer::UpdateOnSkill* data);

	/// MONSTER 
	void ProcessEvent_Monster_Add(NetworkEvent::Game::Event_Monster::Add* data);
	void ProcessEvent_Monster_Remove(NetworkEvent::Game::Event_Monster::Remove* data);
	void ProcessEvent_Monster_Move(NetworkEvent::Game::Event_Monster::Move* data);
	void ProcessEvent_Monster_UpdateHP(NetworkEvent::Game::Event_Monster::UpdateHP* data);
	void ProcessEvent_Monster_UpdateState(NetworkEvent::Game::Event_Monster::UpdateState* data);
	void ProcessEvent_Monster_Target(NetworkEvent::Game::Event_Monster::MonsterTargetUpdate* data);


	long long GetCurrentTimeMilliseconds();
	long long GetTimeStamp();
};

