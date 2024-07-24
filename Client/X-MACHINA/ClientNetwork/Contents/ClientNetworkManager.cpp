#include "stdafx.h"
#include "ClientNetworkManager.h"

#include "Script_Ursacetus.h"
#include "Script_Onyscidus.h"
#include "Script_AdvancedCombatDroid_5.h"

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
#include "ClientNetwork/Contents/Script_RemotePlayer.h"
#include "ClientNetwork/Contents/Script_PlayerNetwork.h"
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

	LOG_MGR->WCout(wifi_Ipv4_wstr, '\n');
	if (FALSE == mClientNetwork->Start(L"192.168.0.12", 7777)) {
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



long long ClientNetworkManager::GetTimeStamp()
{
	return duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}



void ClientNetworkManager::Send(SPtr_PacketSendBuf pkt)
{
	mClientNetwork->Broadcast(pkt);
}

/// +---------------------------------------------------------------------------
/// >> ▶▶▶▶▶ CREATE EVENT 
/// ---------------------------------------------------------------------------+
sptr<NetworkEvent::Game::Event_RemotePlayer::Add> ClientNetworkManager::CreateEvent_Add_RemotePlayer(GamePlayerInfo info)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::Add> Event = std::make_shared<NetworkEvent::Game::Event_RemotePlayer::Add>();
	
	Event->type				= NetworkEvent::Game::RemotePlayerType::Add;

	Event->Id        = info.Id;
	Event->Name      = info.Name;
	Event->Pos       = info.Pos;
	Event->Rot       = info.Rot;
	Event->SpineLook = info.SDir;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::Remove> ClientNetworkManager::CreateEvent_Remove_RemotePlayer(uint32_t remID)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::Remove> Event = std::make_shared<NetworkEvent::Game::Event_RemotePlayer::Remove>();
	
	Event->type = NetworkEvent::Game::RemotePlayerType::Remove;

	Event->Id = remID;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::Move> ClientNetworkManager::CreateEvent_Move_RemotePlayer(uint32_t remID, Vec3 remotePos, ExtData::MOVESTATE movestate )
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::Move> Event = std::make_shared<NetworkEvent::Game::Event_RemotePlayer::Move>();

	Event->type = NetworkEvent::Game::RemotePlayerType::Move;

	Event->Id        = remID;
	Event->Pos       = remotePos;
	Event->MoveState = movestate;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::Extrapolate> ClientNetworkManager::CreateEvent_Extrapolate_RemotePlayer(uint32_t remID, ExtData extdata)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::Extrapolate> Event = std::make_shared<NetworkEvent::Game::Event_RemotePlayer::Extrapolate>();

	Event->type = NetworkEvent::Game::RemotePlayerType::Extrapolate;

	Event->Id                = remID;
	Event->PingTime          = extdata.PingTime;
	Event->ExtPos            = extdata.TargetPos;
	Event->ExtRot            = extdata.TargetRot;
	Event->ExtMoveDir        = extdata.MoveDir;
	Event->MoveState         = extdata.MoveState;
	Event->Velocity          = extdata.Velocity;
	Event->animparam_h       = extdata.Animdata.AnimParam_h;
	Event->animparam_v       = extdata.Animdata.AnimParam_v;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation> ClientNetworkManager::CreateEvent_UpdateAnimation_RemotePlayer(uint32_t remID, int anim_upper_idx, int anim_lower_idx, float anim_param_h, float anim_param_v)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation> Event = std::make_shared<NetworkEvent::Game::Event_RemotePlayer::UpdateAnimation>();

	Event->type = NetworkEvent::Game::RemotePlayerType::UpdateAnimation;
	
	Event->Id            = remID;
	Event->animation_upper_index = static_cast<int32_t>(anim_upper_idx);
	Event->animation_lower_index = static_cast<int32_t>(anim_lower_idx);
	Event->animation_param_h     = anim_param_h;
	Event->animation_param_v     = anim_param_v;

	return Event;
}

sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation> ClientNetworkManager::CreateEvent_UpdateAimRotation_RemotePlayer(uint32_t remID, float aim_rotation_y)
{
	sptr<NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation> Event = std::make_shared<NetworkEvent::Game::Event_RemotePlayer::UpdateAimRotation>();

	Event->type = NetworkEvent::Game::RemotePlayerType::AimRotation;

	Event->id = remID;
	Event->aim_rotation_y = aim_rotation_y;

	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::Add> ClientNetworkManager::CreateEvent_Add_Monster(std::vector<GameMonsterInfo> infos)
{
	sptr<NetworkEvent::Game::Event_Monster::Add> Event = std::make_shared<NetworkEvent::Game::Event_Monster::Add>();

	Event->type = NetworkEvent::Game::MonsterType::Add;
	Event->NewMonsterInfos = infos;

	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::Remove> ClientNetworkManager::CreateEvent_Remove_Monster(std::vector<uint32_t> Ids)
{
	sptr<NetworkEvent::Game::Event_Monster::Remove> Event = std::make_shared<NetworkEvent::Game::Event_Monster::Remove>();
	Event->type = NetworkEvent::Game::MonsterType::Remove;
	Event->IDs = Ids;
	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::Move> ClientNetworkManager::CreateEvent_Move_Monster(std::vector<NetworkEvent::Game::Event_Monster::MonsterMove> infos)
{
	sptr<NetworkEvent::Game::Event_Monster::Move> Event = std::make_shared<NetworkEvent::Game::Event_Monster::Move>();
	Event->type = NetworkEvent::Game::MonsterType::Move;
	Event->Mons = infos;

	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::UpdateHP> ClientNetworkManager::CreateEvent_UpdateHP_Monster(std::vector<NetworkEvent::Game::Event_Monster::MonsterHP> infos)
{
	sptr<NetworkEvent::Game::Event_Monster::UpdateHP> Event = std::make_shared<NetworkEvent::Game::Event_Monster::UpdateHP>();
	Event->type = NetworkEvent::Game::MonsterType::UpdateHP;
	Event->Mons = infos;

	return Event;
}

sptr<NetworkEvent::Game::Event_Monster::UpdateState> ClientNetworkManager::CreateEvent_UpdateState_Monster(std::vector<NetworkEvent::Game::Event_Monster::MonsterUpdateState> infos)
{
	sptr<NetworkEvent::Game::Event_Monster::UpdateState> Event = std::make_shared<NetworkEvent::Game::Event_Monster::UpdateState>();
	Event->type = NetworkEvent::Game::MonsterType::UpdateState;
	Event->Mons = infos;

	return Event;
}



/// +---------------------------------------------------------------------------
/// >> ▶▶▶▶▶ PROCESS EVENT 
/// ---------------------------------------------------------------------------+
void ClientNetworkManager::ProcessEvent_RemotePlayer_Add(NetworkEvent::Game::Event_RemotePlayer::Add* data )
{
	GridObject* remotePlayer = BattleScene::I->Instantiate("EliteTrooper");
	remotePlayer->GetAnimator()->GetController()->SetRemotePlayer();
	remotePlayer->SetName(data->Name);
	remotePlayer->SetID(static_cast<UINT32>(data->Id));
	remotePlayer->SetPosition(data->Pos.x, data->Pos.y, data->Pos.z); /* Position이 이상하면 vector 에러가 날것이다 왜냐? GetHeightTerrain에서 터지기 떄문.. */
	LOG_MGR->Cout("ID : ", data->Id, " POS : ", data->Pos.x, " ", data->Pos.y, " ", data->Pos.z, '\n');

	remotePlayer->AddComponent<Script_RemotePlayer>();


	//Vec4 rot   = remotePlayer->GetRotation();
	//Vec3 euler = Quaternion::ToEuler(rot);
	//euler.y    = data->RemoteP_Rot.y;
	//remotePlayer->SetLocalRotation(Quaternion::ToQuaternion(euler));


	mRemotePlayers[static_cast<UINT32>(data->Id)] = remotePlayer;
	std::cout << "Process Event : Add_RemotePlayer - " << remotePlayer << std::endl;
}

void ClientNetworkManager::ProcessEvent_RemotePlayer_Remove(NetworkEvent::Game::Event_RemotePlayer::Remove* data)
{
	std::cout << "RemoveOtherPlayer \n";
	mRemotePlayers.unsafe_erase(data->Id);
}

void ClientNetworkManager::ProcessEvent_RemotePlayer_Move(NetworkEvent::Game::Event_RemotePlayer::Move* data)
{
	if (mRemotePlayers.count(data->Id)) {
		GridObject* player = mRemotePlayers[data->Id];
		player->GetComponent<Script_RemotePlayer>()->SetPacketPos(data->Pos);
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

	ExtData ExtrapolatedData              = {};
	ExtrapolatedData.PingTime             = data->PingTime;
	ExtrapolatedData.TargetPos            = data->ExtPos;
	ExtrapolatedData.TargetRot            = data->ExtRot;
	ExtrapolatedData.MoveDir              = data->ExtMoveDir;
	ExtrapolatedData.MoveState            = data->MoveState;
	ExtrapolatedData.Velocity             = data->Velocity;
	ExtrapolatedData.Animdata.AnimParam_h = data->animparam_h;
	ExtrapolatedData.Animdata.AnimParam_v = data->animparam_v;

	player->GetComponent<Script_RemotePlayer>()->SetExtrapolatedData(ExtrapolatedData);

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
	player->GetComponent<Script_RemotePlayer>()->RotateTo(data->aim_rotation_y);
	// TODO : Aim Rotation Y 

}

void ClientNetworkManager::ProcessEvent_Monster_Add(NetworkEvent::Game::Event_Monster::Add* data)
{
	std::vector<GameMonsterInfo> monInfos = data->NewMonsterInfos;

	for (int i = 0; i < monInfos.size(); ++i) {
		// Monster 생성! 
		std::string monsterName{};
		switch (monInfos[i].Type) {
		case MonsterType::AdvancedCombatDroid_5:
			monsterName = "AdvancedCombatDroid_5";
			break;
		case MonsterType::Onyscidus:
			monsterName = "Onyscidus";
			break;
		case MonsterType::Ursacetus:
			monsterName = "Ursacetus";
			break;
		default:
			assert(0);
			break;
		}

		GridObject* monster = BattleScene::I->Instantiate(monsterName, ObjectTag::Enemy);
		monster->SetID(monInfos[i].Id);
		monster->SetName(monsterName);
		monster->SetPosition(monInfos[i].Pos);
		monster->SetLocalRotation(monInfos[i].Rot);

		switch (monInfos[i].Type) {
		case MonsterType::AdvancedCombatDroid_5:
			monster->AddComponent<Script_AdvancedCombatDroid_5>();
			break;
		case MonsterType::Onyscidus:
			monster->AddComponent<Script_Onyscidus>();
			break;
		case MonsterType::Ursacetus:
			monster->AddComponent<Script_Ursacetus>();
			break;
		default:
			assert(0);
			break;
		}

		std::cout << "MONSTER ADD ! " << static_cast<uint8_t>(monInfos[i].Type) << " \n";
	}
	

}
void ClientNetworkManager::ProcessEvent_Monster_Remove(NetworkEvent::Game::Event_Monster::Remove* data)
{

}

void ClientNetworkManager::ProcessEvent_Monster_Move(NetworkEvent::Game::Event_Monster::Move* data)
{

}
void ClientNetworkManager::ProcessEvent_Monster_UpdateHP(NetworkEvent::Game::Event_Monster::UpdateHP* data)
{

}
void ClientNetworkManager::ProcessEvent_Monster_UpdateState(NetworkEvent::Game::Event_Monster::UpdateState* data)
{

}

long long ClientNetworkManager::GetCurrentTimeMilliseconds()
{
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}