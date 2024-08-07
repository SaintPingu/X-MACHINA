#include "stdafx.h"
#include "ClientNetworkManager.h"
#include "GameFramework.h"

#include "Script_Ursacetus.h"
#include "Script_Onyscidus.h"
#include "Script_AdvancedCombatDroid_5.h"
#include "Script_Anglerox.h"
#include "Script_Arack.h"
#include "Script_Aranobot.h"
#include "Script_Ceratoferox.h"
#include "Script_Gobbler.h"
#include "Script_LightBipedMech.h"
#include "Script_MiningMech.h"
#include "Script_Rapax.h"
#include "Script_Onyscidus.h"
#include "Script_Rapax.h"
#include "Script_MiningMech.h"
#include "Script_NetworkEnemy.h"
#include "Script_EnemyManager.h"
#include "Script_Player.h"
#include "Script_PlayerController.h"
#include "Script_PheroObject.h"
#include "Script_Phero.h"

#include "Object.h"
#include "BattleScene.h"
#include "Animator.h"
#include "AnimatorController.h"

#include "ClientNetwork/Include/MemoryManager.h"
#include "ClientNetwork/Include/ClientNetwork.h"
#include "ClientNetwork/Contents/ServerSession.h"
#include "ClientNetwork/Include/ThreadManager.h"
#include "ClientNetwork/Include/NetworkManager.h"
#include "ClientNetwork/Include/SendBuffersFactory.h"
#include "ClientNetwork/Contents/Script_NetworkRemotePlayer.h"
#include "ClientNetwork/Contents/Script_NetworkPlayer.h"
#include "ClientNetwork/Contents/Script_NetworkEnemy.h"
#include "ClientNetwork/Contents/SCript_NetworkShootingPlayer.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"

#include "X-Engine.h"

DEFINE_SINGLETON(ClientNetworkManager);
ClientNetworkManager::ClientNetworkManager()
{
}

ClientNetworkManager::~ClientNetworkManager()
{
}

void ClientNetworkManager::Init(std::wstring ip, UINT32 port)
{


	/// +------------------------------------
	///	TLS MGR : Thread Local Storage 관리 
	/// ------------------------------------+
	if (FALSE == TLS_MGR->Init()) {

		LOG_MGR->SetColor(TextColor::Red);
		LOG_MGR->Cout("[FAIL] TLS_MGR INIT\n");
		LOG_MGR->SetColor(TextColor::Default);

	}
	TLS_MGR->Init_TlsInfoData("main Thread TLS Info"); // 0 idx 
	TLS_MGR->Init_TlsSendBufFactory("SendPacketFactory");

	LOG_MGR->Cout("[SUCCESS] TLS_MGR INIT\n");
	/// +----------------------------------------------------------------
///	Network Manager : 2.2버젼 Winsock 초기화 및 비동기 함수 Lpfn 초기화
/// ----------------------------------------------------------------+
	if (FALSE == NETWORK_MGR->Init()) {

		LOG_MGR->SetColor(TextColor::Red);
		LOG_MGR->Cout("[FAIL] NETWORK_MGR INIT\n");
		LOG_MGR->SetColor(TextColor::Default);
	}
	LOG_MGR->Cout("[SUCCESS] NETWORK_MGR INIT\n");

	/// +------------------------
	///	MEMORY : Memory Pool 관리 
	/// ------------------------+
	if (FALSE == MEMORY->InitMemories()) {

		LOG_MGR->SetColor(TextColor::Red);
		LOG_MGR->Cout("[FAIL] MEMORY INIT\n");
		LOG_MGR->SetColor(TextColor::Default);
	}
	LOG_MGR->Cout("[SUCCESS] MEMORY INIT\n");

	/// +-------------------------------------------------------------------
	///	SEND BUFFERS FACTORY : SendBuffer전용 메모리 풀 및 SendPktBuffer 생산
	/// -------------------------------------------------------------------+
	LOG_MGR->Cout("[ING...] ( PLEASE WAIT ) SendBuffersFactory INIT\n");
	{
		SENDBUF_FACTORY->InitPacketMemoryPools();
	}
	LOG_MGR->Cout("[SUCCESS] SendBuffersFactory INIT\n");


	/// +------------------------
	///	  NETWORK SERVICE START  
	/// ------------------------+
	LOG_MGR->Cout("[ING...] ( PLEASE WAIT ) ServerNetwork INIT \n");
	mClientNetwork = Memory::Make_Shared<ClientNetwork>();
	mClientNetwork->SetMaxSessionCnt(1); // 1명 접속  
	mClientNetwork->SetSessionConstructorFunc(std::make_shared<ServerSession>);

	std::string Wifi_Ipv4 = GetLocalIPv4Address();
	std::wstring wifi_Ipv4_wstr = StringToWstring(Wifi_Ipv4);

	//#define USE_LOOP_BACK_ADDR
#ifdef USE_LOOP_BACK_ADDR
	wifi_Ipv4_wstr = L"127.0.0.1";
#endif

	LOG_MGR->WCout("My IP : ", wifi_Ipv4_wstr, '\n');
	std::string filePath = "ClientNetwork/ServerIP.txt";
	std::string serverIP = GetServerIPFromtxt(filePath);
	std::wstring WserverIP = std::wstring(serverIP.begin(), serverIP.end());
	if (FALSE == mClientNetwork->Start(WserverIP, 7777)) {
		LOG_MGR->Cout("CLIENT NETWORK SERVICE START FAIL\n");
		return;
	}
	LOG_MGR->Cout("[SUCCESS] CLIENT NETWORK SERVICE START \n");
}

void ClientNetworkManager::Launch(int ThreadNum)
{

	LOG_MGR->SetColor(TextColor::BrightCyan);
	LOG_MGR->Cout("+--------------------------------------\n");
	LOG_MGR->Cout("       X-MACHINA CLIENT NETWORK        \n");
	LOG_MGR->Cout("--------------------------------------+\n");
	LOG_MGR->SetColor(TextColor::Default);

	mIsRunning = true;
	for (int i = 1; i <= ThreadNum; ++i) {
		std::string ThreadName = "Network Thread_" + std::to_string(i);
		THREAD_MGR->RunThread(ThreadName, [&]() {
			while (mIsRunning) {
				mClientNetwork->Dispatch_CompletedTasks_FromIOCP(0);
			}
			});
	}

	/* Join은 GameFramework에서 ... */
}

void ClientNetworkManager::Stop()
{
	mIsRunning = false;
	THREAD_MGR->JoinAllThreads();
}

void ClientNetworkManager::ProcessEvents()
{
	SwapEventsQueue();
	int FrontIdx = mFrontSceneEventIndex.load();

	while (!mSceneEvnetQueue[FrontIdx].EventsQueue.empty()) {
		sptr<NetworkEvent::Game::EventData> EventData = nullptr;

		mSceneEvnetQueue[FrontIdx].EventsQueue.try_pop(EventData);
		if (EventData == nullptr) continue;
		//LOG_MGR->Cout("EVENT TYPE : (1-Add) (2-Mov) (3-Rem)", EventData->type, "\n");

		switch (EventData->type)
		{
			/// +---------------------------------------------------------------------------
			/// >> ▶▶▶▶▶ PROCESS EVENT REMOTE PLAYER 
			/// ---------------------------------------------------------------------------+
		case NetworkEvent::Game::RemotePlayerType::Add:
		{
			NetworkEvent::Game::Event_RemotePlayer::Add* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::Add*>(EventData.get());
			ProcessEvent_RemotePlayer_Add(data);
		}
		break;
		case NetworkEvent::Game::RemotePlayerType::Move:
		{

			NetworkEvent::Game::Event_RemotePlayer::Move* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::Move*>(EventData.get());
			ProcessEvent_RemotePlayer_Move(data);
		}
		break;
		case NetworkEvent::Game::RemotePlayerType::Remove:
		{
			NetworkEvent::Game::Event_RemotePlayer::Remove* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::Remove*>(EventData.get());
			ProcessEvent_RemotePlayer_Remove(data);
		}
		break;
		case NetworkEvent::Game::RemotePlayerType::Extrapolate:
		{
			NetworkEvent::Game::Event_RemotePlayer::Extrapolate* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::Extrapolate*>(EventData.get());
			ProcessEvent_RemotePlayer_Extrapolate(data);
		}
		break;
		case NetworkEvent::Game::RemotePlayerType::UpdateAnimation:
		{
			NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation*>(EventData.get());
			ProcessEvent_RemotePlayer_UpdateAnimation(data);
		}
		break;
		case NetworkEvent::Game::RemotePlayerType::AimRotation:
		{
			NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation*>(EventData.get());
			ProcessEvent_RemotePlayer_AimRotation(data);
		}
		break;
		case NetworkEvent::Game::RemotePlayerType::UpdateWeapon:
		{
			NetworkEvent::Game::Event_RemotePlayer::UpdateWeapon* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::UpdateWeapon*>(EventData.get());
			ProcessEvent_RemotePlayer_UpdateWeapon(data);
		}
		break;
		case NetworkEvent::Game::RemotePlayerType::OnShoot:
		{
			NetworkEvent::Game::Event_RemotePlayer::UpdateOnShoot* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::UpdateOnShoot*>(EventData.get());
			ProcessEvent_RemotePlayer_UpdateOnShoot(data);
		}
		break;
		case NetworkEvent::Game::RemotePlayerType::OnSkill:
		{
			NetworkEvent::Game::Event_RemotePlayer::UpdateOnSkill* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::UpdateOnSkill*>(EventData.get());
			ProcessEvent_RemotePlayer_UpdateOnSkill(data);
		}
		break;
		case NetworkEvent::Game::RemotePlayerType::State:
		{
			NetworkEvent::Game::Event_RemotePlayer::UpdateState* data = reinterpret_cast<NetworkEvent::Game::Event_RemotePlayer::UpdateState*>(EventData.get());
			ProcessEvent_RemotePlayer_UpdateState(data);
		}
		break;



		/// +---------------------------------------------------------------------------
		/// >> ▶▶▶▶▶ PROCESS EVENT MONSTER  
		/// ---------------------------------------------------------------------------+
		case NetworkEvent::Game::MonsterType::Add:
		{
			NetworkEvent::Game::Event_Monster::Add* data = reinterpret_cast<NetworkEvent::Game::Event_Monster::Add*>(EventData.get());
			ProcessEvent_Monster_Add(data);
		}
		break;
		case NetworkEvent::Game::MonsterType::Remove:
		{
			NetworkEvent::Game::Event_Monster::Remove* data = reinterpret_cast<NetworkEvent::Game::Event_Monster::Remove*>(EventData.get());
			ProcessEvent_Monster_Remove(data);
		}
		break;
		case NetworkEvent::Game::MonsterType::Dead:
		{
			NetworkEvent::Game::Event_Monster::MonsterDead* data = reinterpret_cast<NetworkEvent::Game::Event_Monster::MonsterDead*>(EventData.get());
			ProcessEvent_Monster_Dead(data);
		}
		break;

		case NetworkEvent::Game::MonsterType::Move:
		{
			NetworkEvent::Game::Event_Monster::Move* data = reinterpret_cast<NetworkEvent::Game::Event_Monster::Move*>(EventData.get());
			ProcessEvent_Monster_Move(data);
		}
		break;
		case NetworkEvent::Game::MonsterType::UpdateHP:
		{
			NetworkEvent::Game::Event_Monster::UpdateHP* data = reinterpret_cast<NetworkEvent::Game::Event_Monster::UpdateHP*>(EventData.get());
			ProcessEvent_Monster_UpdateHP(data);
		}
		break;
		case NetworkEvent::Game::MonsterType::UpdateState:
		{
			NetworkEvent::Game::Event_Monster::UpdateState* data = reinterpret_cast<NetworkEvent::Game::Event_Monster::UpdateState*>(EventData.get());
			ProcessEvent_Monster_UpdateState(data);
		}
		break;
		case NetworkEvent::Game::MonsterType::Target:
		{
			NetworkEvent::Game::Event_Monster::MonsterTargetUpdate* data = reinterpret_cast<NetworkEvent::Game::Event_Monster::MonsterTargetUpdate*>(EventData.get());
			ProcessEvent_Monster_Target(data);
		}
		break;
		/// +---------------------------------------------------------------------------
		/// >> ▶▶▶▶▶ PROCESS EVENT Phero
		/// ---------------------------------------------------------------------------+
		case NetworkEvent::Game::PheroType::GetPhero:
		{
			NetworkEvent::Game::Event_Phero::GetPhero* data = reinterpret_cast<NetworkEvent::Game::Event_Phero::GetPhero*>(EventData.get());
			ProcessEvent_Phero_Get(data);
		}
		break;
		/// +---------------------------------------------------------------------------
		/// >> ▶▶▶▶▶ PROCESS EVENT Contents
		/// ---------------------------------------------------------------------------+
		case NetworkEvent::Game::ContentsType::Chat:
		{
			NetworkEvent::Game::Event_Contents::Chat* data = reinterpret_cast<NetworkEvent::Game::Event_Contents::Chat*>(EventData.get());
			ProcessEvent_Contents_Chat(data);
		}
		break;
		}
	}
}

void ClientNetworkManager::SwapEventsQueue()
{
	mFrontSceneEventIndex.store((mFrontSceneEventIndex == 0) ? 1 : 0);
	mBackSceneEventIndex.store((mBackSceneEventIndex == 0) ? 1 : 0);

}

void ClientNetworkManager::RegisterEvent(sptr<NetworkEvent::Game::EventData> data)
{
	mSceneEvnetQueue[mBackSceneEventIndex.load()].EventsQueue.push(data);
}

std::string ClientNetworkManager::GetLocalIPv4Address()
{
	std::string ipAddress;

	// 초기화
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 네트워크 인터페이스 정보 가져오기
	ULONG bufferSize = 0;
	if (GetAdaptersAddresses(AF_INET, 0, nullptr, nullptr, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
		std::vector<char> buffer(bufferSize);
		PIP_ADAPTER_ADDRESSES addressesBuffer = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&buffer[0]);

		if (GetAdaptersAddresses(AF_INET, 0, nullptr, addressesBuffer, &bufferSize) == NO_ERROR) {
			for (PIP_ADAPTER_ADDRESSES adapter = addressesBuffer; adapter != nullptr; adapter = adapter->Next) {
				if (adapter->IfType == IF_TYPE_IEEE80211 && adapter->OperStatus == IfOperStatusUp) {
					IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress;
					if (unicast != nullptr) {
						sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(unicast->Address.lpSockaddr);
						char ipBuffer[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, &(addr->sin_addr), ipBuffer, INET_ADDRSTRLEN);
						ipAddress = ipBuffer;
						break; // 첫 번째 IP만 필요하므로 루프 종료
					}
				}
			}
		}
	}

	WSACleanup();
	return ipAddress;
}

GridObject* ClientNetworkManager::GetRemotePlayer(UINT32 id)
{
	GridObject* player = nullptr;
	if (mRemotePlayers.count(id)) {
		player = mRemotePlayers[id];
	}
	return player;
}



long long ClientNetworkManager::GetTimeStamp()
{
	return duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}



void ClientNetworkManager::Send(SPtr_PacketSendBuf pkt)
{
	mClientNetwork->Broadcast(pkt);
}

sptr<NetworkEvent::Game::Event_RemotePlayer::Add> ClientNetworkManager::CreateEvent_Add_RemotePlayer(GamePlayerInfo info)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::Add> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_RemotePlayer::Add>();

	Event->type = NetworkEvent::Game::RemotePlayerType::Add;

	Event->Id = info.Id;
	Event->Name = info.Name;
	Event->Pos = info.Pos;
	Event->Rot = info.Rot;
	Event->SpineLook = info.SDir;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::Remove> ClientNetworkManager::CreateEvent_Remove_RemotePlayer(uint32_t remID)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::Remove> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_RemotePlayer::Remove>();

	Event->type = NetworkEvent::Game::RemotePlayerType::Remove;

	Event->Id = remID;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::Move> ClientNetworkManager::CreateEvent_Move_RemotePlayer(uint32_t remID, Vec3 remotePos, ExtData::MOVESTATE movestate)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::Move> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_RemotePlayer::Move>();

	Event->type = NetworkEvent::Game::RemotePlayerType::Move;

	Event->Id = remID;
	Event->Pos = remotePos;
	Event->MoveState = movestate;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::Extrapolate> ClientNetworkManager::CreateEvent_Extrapolate_RemotePlayer(uint32_t remID, ExtData extdata)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::Extrapolate> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_RemotePlayer::Extrapolate>();

	Event->type = NetworkEvent::Game::RemotePlayerType::Extrapolate;

	Event->Id = remID;
	Event->PingTime = extdata.PingTime;
	Event->ExtPos = extdata.TargetPos;
	Event->ExtRot = extdata.TargetRot;
	Event->ExtMoveDir = extdata.MoveDir;
	Event->MoveState = extdata.MoveState;
	Event->Velocity = extdata.Velocity;
	Event->animparam_h = extdata.Animdata.AnimParam_h;
	Event->animparam_v = extdata.Animdata.AnimParam_v;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation> ClientNetworkManager::CreateEvent_UpdateAnimation_RemotePlayer(uint32_t remID, int anim_upper_idx, int anim_lower_idx, float anim_param_h, float anim_param_v)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation>();

	Event->type = NetworkEvent::Game::RemotePlayerType::UpdateAnimation;

	Event->Id = remID;
	Event->animation_upper_index = static_cast<int32_t>(anim_upper_idx);
	Event->animation_lower_index = static_cast<int32_t>(anim_lower_idx);
	Event->animation_param_h = anim_param_h;
	Event->animation_param_v = anim_param_v;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation> ClientNetworkManager::CreateEvent_UpdateAimRotation_RemotePlayer(uint32_t remID, float aim_rotation_y, float spine_angle)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation> Event = std::make_shared<NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation>();

	Event->type = NetworkEvent::Game::RemotePlayerType::AimRotation;

	Event->id = remID;
	Event->aim_rotation_y = aim_rotation_y;
	Event->spine_angle = spine_angle;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateWeapon> ClientNetworkManager::CreateEvent_UpdateWeapon_RemotePlayer(uint32_t remID, FBProtocol::WEAPON_TYPE weaponType)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateWeapon> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_RemotePlayer::UpdateWeapon>();

	Event->type = NetworkEvent::Game::RemotePlayerType::UpdateWeapon;

	Event->Id = remID;

	Event->weapon_type = weaponType;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateOnShoot> ClientNetworkManager::CreateEvent_UpdateOnShoot_RemotePlayer(uint32_t remID, int bullet_id, int weapon_id, Vec3 ray)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateOnShoot> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_RemotePlayer::UpdateOnShoot>();

	Event->type = NetworkEvent::Game::RemotePlayerType::OnShoot;

	Event->id = remID;
	Event->bullet_id = bullet_id;
	Event->weapon_id = weapon_id;
	Event->ray = ray;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateOnSkill> ClientNetworkManager::CreateEvent_UpdateOnSkill_RemotePlayer(uint32_t remID, FBProtocol::PLAYER_SKILL_TYPE skillType, float phero_amount, int mindControl_monster_id)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateOnSkill> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_RemotePlayer::UpdateOnSkill>();

	Event->type = NetworkEvent::Game::RemotePlayerType::OnSkill;

	Event->id = remID;
	Event->skill_type = skillType;
	Event->phero_amount = phero_amount;
	Event->mindControl_monster_id = mindControl_monster_id;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateState> ClientNetworkManager::CreateEvent_UpdateState_RemotePlayer(uint32_t remID, FBProtocol::PLAYER_STATE_TYPE state_Type, float hp, float phero)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateState> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_RemotePlayer::UpdateState>();

	Event->type = NetworkEvent::Game::RemotePlayerType::State;

	Event->id = remID;
	Event->hp = hp;
	Event->phero = phero;
	Event->state_type = state_Type;


	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::Add> ClientNetworkManager::CreateEvent_Add_Monster(std::vector<GameMonsterInfo> infos)
{
	sptr<NetworkEvent::Game::Event_Monster::Add> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_Monster::Add>();

	Event->type = NetworkEvent::Game::MonsterType::Add;
	Event->NewMonsterInfos = infos;

	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::Remove> ClientNetworkManager::CreateEvent_Remove_Monster(std::vector<uint32_t> Ids)
{
	sptr<NetworkEvent::Game::Event_Monster::Remove> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_Monster::Remove>();
	Event->type = NetworkEvent::Game::MonsterType::Remove;
	Event->IDs = Ids;
	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::MonsterDead> ClientNetworkManager::CreateEvent_Dead_Monster(uint32_t monster_id, Vec3 dead_point, std::string pheros)
{
	sptr<NetworkEvent::Game::Event_Monster::MonsterDead> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_Monster::MonsterDead>();
	Event->type					= NetworkEvent::Game::MonsterType::Dead;
	Event->monster_id			= monster_id;
	Event->monster_dead_point	= dead_point;
	Event->pheros				= pheros;

	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::Move> ClientNetworkManager::CreateEvent_Move_Monster(std::vector<NetworkEvent::Game::Event_Monster::MonsterMove> infos)
{
	sptr<NetworkEvent::Game::Event_Monster::Move> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_Monster::Move>();
	Event->type = NetworkEvent::Game::MonsterType::Move;
	Event->Mons = infos;

	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::UpdateHP> ClientNetworkManager::CreateEvent_UpdateHP_Monster(std::vector<NetworkEvent::Game::Event_Monster::MonsterHP> infos)
{
	sptr<NetworkEvent::Game::Event_Monster::UpdateHP> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_Monster::UpdateHP>();
	Event->type = NetworkEvent::Game::MonsterType::UpdateHP;
	Event->Mons = infos;

	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::UpdateState> ClientNetworkManager::CreateEvent_UpdateState_Monster(std::vector<NetworkEvent::Game::Event_Monster::MonsterUpdateState> infos)
{
	sptr<NetworkEvent::Game::Event_Monster::UpdateState> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_Monster::UpdateState>();
	Event->type = NetworkEvent::Game::MonsterType::UpdateState;
	Event->Mons = infos;

	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::MonsterTargetUpdate> ClientNetworkManager::CreateEvent_Monster_Target(std::vector<NetworkEvent::Game::Event_Monster::MonsterTarget> infos)
{
	sptr<NetworkEvent::Game::Event_Monster::MonsterTargetUpdate> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_Monster::MonsterTargetUpdate>();
	Event->type = NetworkEvent::Game::MonsterType::Target;
	Event->Mons = infos;

	return Event;
}

sptr<NetworkEvent::Game::Event_Phero::GetPhero> ClientNetworkManager::CreateEvent_GetPhero(uint32_t player_id, uint32_t phero_id)
{
	sptr<NetworkEvent::Game::Event_Phero::GetPhero> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_Phero::GetPhero>();
	Event->type = NetworkEvent::Game::PheroType::GetPhero;
	
	Event->phero_id		= phero_id;
	Event->player_id	= player_id;
	
	return Event;
}

sptr<NetworkEvent::Game::Event_Contents::Chat> ClientNetworkManager::CreateEvent_Chat(uint32_t Id, std::string chat)
{
	sptr<NetworkEvent::Game::Event_Contents::Chat> Event = MEMORY->Make_Shared<NetworkEvent::Game::Event_Contents::Chat>();
	
	Event->type = NetworkEvent::Game::ContentsType::Chat;
	
	Event->Id	= Id;
	Event->chat = chat;

	return Event;
}


/// +---------------------------------------------------------------------------
/// >> ▶▶▶▶▶ PROCESS EVENT 
/// ---------------------------------------------------------------------------+
void ClientNetworkManager::ProcessEvent_RemotePlayer_Add(NetworkEvent::Game::Event_RemotePlayer::Add* data)
{
	if (data->Name == "MyPlayer") {
		mRemotePlayers[static_cast<UINT32>(data->Id)] = GameFramework::I->GetPlayer();
		return;
	}

	GridObject* remotePlayer = BattleScene::I->Instantiate("EliteTrooper");
	remotePlayer->GetAnimator()->GetController()->SetRemotePlayer();
	remotePlayer->SetName(data->Name);
	remotePlayer->SetID(static_cast<UINT32>(data->Id));
	remotePlayer->SetPosition(data->Pos.x, data->Pos.y, data->Pos.z); /* Position이 이상하면 vector 에러가 날것이다 왜냐? GetHeightTerrain에서 터지기 떄문.. */
	LOG_MGR->Cout("ID : ", data->Id, " POS : ", data->Pos.x, " ", data->Pos.y, " ", data->Pos.z, '\n');

	remotePlayer->AddComponent<Script_NetworkRemotePlayer>();

	mRemotePlayers[static_cast<UINT32>(data->Id)] = remotePlayer;
	//std::cout << "Process Event : Add_RemotePlayer - " << remotePlayer << std::endl;
}

void ClientNetworkManager::ProcessEvent_RemotePlayer_Remove(NetworkEvent::Game::Event_RemotePlayer::Remove* data)
{
	std::cout << "RemoveOtherPlayer \n";

	if (!mRemotePlayers.count(data->Id)) {
		return;
	}

	BattleScene::I->RemoveDynamicObject(mRemotePlayers[data->Id]);
	mRemotePlayers.unsafe_erase(data->Id);
}

void ClientNetworkManager::ProcessEvent_RemotePlayer_Move(NetworkEvent::Game::Event_RemotePlayer::Move* data)
{
	if (mRemotePlayers.count(data->Id)) {
		GridObject* player = mRemotePlayers[data->Id];
		player->GetComponent<Script_NetworkRemotePlayer>()->SetPacketPos(data->Pos);
		//player->SetPosition(data->RemoteP_Pos);
	}
	else {
		LOG_MGR->Cout("Player - ", data->Id, "Not Existed\n");
	}
}

void ClientNetworkManager::ProcessEvent_RemotePlayer_Extrapolate(NetworkEvent::Game::Event_RemotePlayer::Extrapolate* data)
{
	if (!mRemotePlayers.count(data->Id)) {
		return;
	}

	GridObject* player = mRemotePlayers[data->Id];

	ExtData ExtrapolatedData = {};
	ExtrapolatedData.PingTime = data->PingTime;
	ExtrapolatedData.TargetPos = data->ExtPos;
	ExtrapolatedData.TargetRot = data->ExtRot;
	ExtrapolatedData.MoveDir = data->ExtMoveDir;
	ExtrapolatedData.MoveState = data->MoveState;
	ExtrapolatedData.Velocity = data->Velocity;
	ExtrapolatedData.Animdata.AnimParam_h = data->animparam_h;
	ExtrapolatedData.Animdata.AnimParam_v = data->animparam_v;

	if ((player->GetPosition() - ExtrapolatedData.TargetPos).Length() >= 5.f) {
		player->SetPosition(ExtrapolatedData.TargetPos);
	}

	player->GetComponent<Script_NetworkRemotePlayer>()->SetExtrapolatedData(ExtrapolatedData);

}

void ClientNetworkManager::ProcessEvent_RemotePlayer_UpdateAnimation(NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation* data)
{
	if (!mRemotePlayers.count(data->Id)) {
		return;
	}
	GridObject* player = mRemotePlayers[data->Id];
	player->GetAnimator()->GetController()->SetAnimation(data->animation_upper_index, data->animation_lower_index, data->animation_param_v, data->animation_param_h);

}

void ClientNetworkManager::ProcessEvent_RemotePlayer_AimRotation(NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation* data)
{
	if (!mRemotePlayers.count(data->id)) {
		return;
	}

	GridObject* player = mRemotePlayers[data->id];
	player->GetComponent<Script_NetworkRemotePlayer>()->RotateTo(data->aim_rotation_y, data->spine_angle);
}

void ClientNetworkManager::ProcessEvent_RemotePlayer_UpdateWeapon(NetworkEvent::Game::Event_RemotePlayer::UpdateWeapon* data)
{
	uint32_t				player_id = data->Id;
	FBProtocol::WEAPON_TYPE weapon_Type = data->weapon_type;

	if (!mRemotePlayers.count(data->Id)) {
		return;
	}

	GridObject* player = mRemotePlayers[player_id];
	auto script_NRP = player->GetComponent<Script_NetworkRemotePlayer>();
	if (script_NRP) {
		script_NRP->SetCurrWeaponName(weapon_Type);
	}

	//  TODO: Remote PLayer 가 무기를 들게 한다. 

}

std::string ClientNetworkManager::GetServerIPFromtxt(const std::string& filePath)
{
	std::ifstream file(filePath);

	// 파일이 열리지 않으면 빈 문자열 반환
	if (!file.is_open()) {
		std::cerr << "파일을 열 수 없습니다: " << filePath << std::endl;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf(); // 파일 내용을 버퍼에 저장

	return buffer.str(); // 문자열 반환
}

void ClientNetworkManager::ProcessEvent_RemotePlayer_UpdateOnShoot(NetworkEvent::Game::Event_RemotePlayer::UpdateOnShoot* data)
{
	uint32_t	player_id = data->id;
	int			bullet_id = data->bullet_id;
	int			weapon_id = data->weapon_id;
	Vec3		ray = data->ray; // Remote Player 가 발사한 총알 방향  

	GridObject* player = mRemotePlayers[player_id];
	const auto& script_NRP = player->GetComponent<Script_NetworkRemotePlayer>();
	if (script_NRP) {

		// 현재 RemotePlayer 가 들고 있는 무기 이름 
		WeaponName currWeaponName = script_NRP->GetCurrWeaponName();
		script_NRP->FireBullet();
	}
	// TODO: Remote PLayer 가 총을 쏘게 한다

	LOG_MGR->Cout(player_id, " OnShoot : ", ray.x, " ", ray.y, " ", ray.z, '\n');



}

void ClientNetworkManager::ProcessEvent_RemotePlayer_UpdateOnSkill(NetworkEvent::Game::Event_RemotePlayer::UpdateOnSkill* data)
{

	int player_id = data->id;
	FBProtocol::PLAYER_SKILL_TYPE skill_type = data->skill_type;
	int mindControl_monster_id = data->mindControl_monster_id;

	LOG_MGR->Cout(player_id, " OnSkill : ", static_cast<int>(skill_type), '\n');

}

void ClientNetworkManager::ProcessEvent_RemotePlayer_UpdateState(NetworkEvent::Game::Event_RemotePlayer::UpdateState* data)
{
	data->id;
	data->hp;
	data->phero;
	data->state_type;

}

void ClientNetworkManager::ProcessEvent_Monster_Add(NetworkEvent::Game::Event_Monster::Add* data)
{
	/*
		BattelScene ----- [1] [2] [3]
		[2]


	*/
	std::vector<GameMonsterInfo> monInfos = data->NewMonsterInfos;

	for (int i = 0; i < monInfos.size(); ++i) {

		int							monsterID = monInfos[i].Id;
		FBProtocol::MONSTER_TYPE	monType = monInfos[i].Type;
		FBProtocol::MONSTER_BT_TYPE monBtType = monInfos[i].bt_type;
		std::vector<PheroInfo>		pheros = monInfos[i].mPheros;
		std::string					name = monInfos[i].Name;
		Vec3						position = monInfos[i].Pos;
		Vec4						Rotation = monInfos[i].Rot;
		Vec3						SpineDir = monInfos[i].SDir;


		// 몬스터가 이미 생성된 적이 있다면 
		if (mRemoteMonsters.count(monsterID)) {
			mRemoteMonsters[monsterID]->SetActiveMyObject(true);
			mRemoteMonsters[monsterID]->SetState(monBtType);
			mRemoteMonsters[monsterID]->SetPosition(position);
			mRemoteMonsters[monsterID]->SetLocalRotation(Rotation);

			if (monInfos[i].Target_Player_Id == 0) {
				mRemoteMonsters[monsterID]->SetTarget(nullptr);
			}
			else {
				mRemoteMonsters[monsterID]->SetTarget(mRemotePlayers[monInfos[i].Target_Player_Id]);
			}

			return;
		}

		// Monster 생성! 
		std::string MonsterTypeNames[FBProtocol::MONSTER_TYPE_MAX + 1] = {
					"AdvancedCombatDroid",
					"Anglerox",
					"Arack",
					"Aranobot",
					"Ceratoferox",
					"Gobbler",
					"LightBipedMech",
					"MiningMech",
					"Onyscidus",
					"Ursacetus",
					"Rapax"
		};


		std::string monsterName = MonsterTypeNames[monInfos[i].Type];


		GridObject* monster = BattleScene::I->Instantiate(monsterName, ObjectTag::Enemy);
		monster->SetID(monInfos[i].Id);
		monster->SetName(monsterName);
		monster->SetPosition(monInfos[i].Pos);
		monster->SetLocalRotation(monInfos[i].Rot);

		switch (monInfos[i].Type) {
		case FBProtocol::MONSTER_TYPE_ADVANCED_COMBAT_DROIR_5:
			monster->AddComponent<Script_AdvancedCombatDroid_5>();
			break;
		case FBProtocol::MONSTER_TYPE_ONYSCIDUS:
			monster->AddComponent<Script_Onyscidus>();
			break;
		case FBProtocol::MONSTER_TYPE_URSACETUS:
			monster->AddComponent<Script_Ursacetus>();
			break;
		case FBProtocol::MONSTER_TYPE_ANGLEROX:
			monster->AddComponent<Script_Anglerox>();
			break;
		case FBProtocol::MONSTER_TYPE_ARACK:
			monster->AddComponent<Script_Arack>();
			break;
		case FBProtocol::MONSTER_TYPE_ARANOBOT:
			monster->AddComponent<Script_Aranobot>();
			break;
		case FBProtocol::MONSTER_TYPE_CERATOFEROX:
			monster->AddComponent<Script_Ceratoferox>();
			break;
		case FBProtocol::MONSTER_TYPE_GOBBLER:
			monster->AddComponent<Script_Gobbler>();
			break;
		case FBProtocol::MONSTER_TYPE_MININGMECH:
			monster->AddComponent<Script_MiningMech>();
			break;
		case FBProtocol::MONSTER_TYPE_LIGHTBIPEDMECH:
			monster->AddComponent<Script_LightBipedMech>();
			break;
		case FBProtocol::MONSTER_TYPE_RAPAX:
			monster->AddComponent<Script_Rapax>();
			break;
		default:
			assert(0);
			break;
		}
		Script_NetworkEnemy* enemyNetwork = monster->AddComponent<Script_NetworkEnemy>().get();
		//enemyNetwork->SetPheroInfo(monInfos[i].mPheros);

		// 들어온 몬스터를 관리하기 위해 mRemoteMonsters 에 집어 넣는다. 
		if (monInfos[i].Target_Player_Id == 0) {
			enemyNetwork->SetTarget(nullptr);
		}
		else {
			enemyNetwork->SetTarget(mRemotePlayers[monInfos[i].Target_Player_Id]);
		}

		mRemoteMonsters.insert(std::make_pair(monInfos[i].Id, enemyNetwork));
		mRemoteMonsters[monsterID]->SetState(monBtType);

		//std::cout << "MONSTER ADD ! " << static_cast<uint8_t>(monInfos[i].Type) << " \n";
	}


}
void ClientNetworkManager::ProcessEvent_Monster_Remove(NetworkEvent::Game::Event_Monster::Remove* data)
{
	for (int id : data->IDs) {
		if (!mRemoteMonsters.count(id)) {
			continue;
		}

		mRemoteMonsters[id]->SetActiveMyObject(false);
	}
}

void ClientNetworkManager::ProcessEvent_Monster_Dead(NetworkEvent::Game::Event_Monster::MonsterDead* data)
{
	const Vec3 deadPoint = data->monster_dead_point;
	const int monsterId = data->monster_id;
	const std::string pheros = data->pheros;

	std::vector<int> pheroCounts(PheroDropInfo::gkMaxLevel + 1);
	for (char phero : pheros) {
		pheroCounts[phero - '0']++;
	}

	int pheroIdx{};
	for (int level = 1; level <= PheroDropInfo::gkMaxLevel; ++level) {
		for (auto& phero : Script_PheroObject::GeneratePheroPool(level, pheroCounts[level], deadPoint, monsterId, pheroIdx)) {
			mRemotePheros.insert({phero->GetID(), phero});
		}
	}
}

void ClientNetworkManager::ProcessEvent_Monster_Move(NetworkEvent::Game::Event_Monster::Move* data)
{
	for (int i = 0; i < data->Mons.size(); ++i) {
		int		ID = data->Mons[i].Id;
		Vec3	Pos = data->Mons[i].Pos;
		float	Angle = data->Mons[i].Angle;

		if (!mRemoteMonsters.count(ID))
			continue;

		/*if ((Pos - mRemoteMonsters[ID]->GetPosition()).Length() >= 1.f)
			mRemoteMonsters[ID]->SetPosition(Pos);*/
		mRemoteMonsters[ID]->SetPosition(Pos);

		mRemoteMonsters[ID]->SetRotation(Angle);
		//mRemoteMonsters[ID]->SetTarget(nullptr);
		//mRemoteMonsters[ID]->SetState(EnemyState::Idle);
	}
}
void ClientNetworkManager::ProcessEvent_Monster_UpdateHP(NetworkEvent::Game::Event_Monster::UpdateHP* data)
{

}
void ClientNetworkManager::ProcessEvent_Monster_UpdateState(NetworkEvent::Game::Event_Monster::UpdateState* data)
{
	for (int i = 0; i < data->Mons.size(); ++i) {
		uint32_t					ID = data->Mons[i].Id;
		FBProtocol::MONSTER_BT_TYPE type = data->Mons[i].state;

		if (!mRemoteMonsters.count(ID))
			continue;

		if (mRemoteMonsters[ID]->GetState() == EnemyState::Death) {
			continue;
		}

		mRemoteMonsters[ID]->SetState(type);
	}
}

void ClientNetworkManager::ProcessEvent_Monster_Target(NetworkEvent::Game::Event_Monster::MonsterTargetUpdate* data)
{
	for (int i = 0; i < data->Mons.size(); ++i) {

		int monster_id = data->Mons[i].id;
		int target_monster_id = data->Mons[i].target_monster_id;
		int target_player_id = data->Mons[i].target_player_id;
		if (!mRemoteMonsters.count(monster_id))
			continue;

		if (!mRemotePlayers.count(target_player_id))
			continue;

		//std::cout << "target : " << target_player_id << "\n";
		if (target_player_id == 0) {
			mRemoteMonsters[monster_id]->SetTarget(nullptr);
		}
		else {
			mRemoteMonsters[monster_id]->SetTarget(mRemotePlayers[target_player_id]);
		}
	}
}

void ClientNetworkManager::ProcessEvent_Phero_Get(NetworkEvent::Game::Event_Phero::GetPhero* data)
{
	UINT32 pheroID = data->phero_id;
	UINT32 playerID = data->player_id;

	if (!mRemotePheros.count(pheroID)) {
		std::cout << "No pheroID : " << pheroID << ", ";
		return;
	}

	if (!mRemotePlayers.count(playerID)) {
		return;
	}

	std::cout << "pheroID : " << pheroID << ", ";
	std::cout << "targetID : " << playerID << "\n";

	mRemotePheros[pheroID]->SetTarget(mRemotePlayers[playerID]);
}

void ClientNetworkManager::ProcessEvent_Contents_Chat(NetworkEvent::Game::Event_Contents::Chat* data)
{
	std::string chat	= data->chat;
	uint32_t player_id	= data->Id;

	GameFramework::I->GetPlayerScript()->Chat(chat);
}


long long ClientNetworkManager::GetCurrentTimeMilliseconds()
{
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
