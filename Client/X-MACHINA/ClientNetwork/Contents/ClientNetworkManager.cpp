#include "stdafx.h"
#include "ClientNetworkManager.h"

#include "Script_GroundObject.h"

#include "Object.h"
#include "Scene.h"
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

	if (FALSE == mClientNetwork->Start(L"192.168.0.11", 7777)) {
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

	for (int i = 1; i <= ThreadNum; ++i) {
		std::string ThreadName = "Network Thread_" + std::to_string(i);
		THREAD_MGR->RunThread(ThreadName, [&]() {
			while (true) {
				mClientNetwork->Dispatch_CompletedTasks_FromIOCP(0);
			}
			});
	}

	/* Join은 GameFramework에서 ... */
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
		case NetworkEvent::Game::Enum::Add_RemotePlayer:
		{

			NetworkEvent::Game::Add_RemotePlayer* data = reinterpret_cast<NetworkEvent::Game::Add_RemotePlayer*>(EventData.get());

			sptr<GridObject> remotePlayer = Scene::I->Instantiate("EliteTrooper");
			remotePlayer->SetName(data->RemoteP_Name);
			remotePlayer->SetID(static_cast<UINT32>(data->RemoteP_ID));

			remotePlayer->AddComponent<Script_RemotePlayer>();
			remotePlayer->AddComponent<Script_GroundObject>();
			
			remotePlayer->SetPosition(data->RemoteP_Pos.x, data->RemoteP_Pos.y, data->RemoteP_Pos.z); /* Position이 이상하면 vector 에러가 날것이다 왜냐? GetHeightTerrain에서 터지기 떄문.. */
			LOG_MGR->Cout("ID : ", data->RemoteP_ID, " POS : ", data->RemoteP_Pos.x, " ", data->RemoteP_Pos.y, " ", data->RemoteP_Pos.z, '\n');

			//Vec4 rot   = remotePlayer->GetRotation();
			//Vec3 euler = Quaternion::ToEuler(rot);
			//euler.y    = data->RemoteP_Rot.y;
			//remotePlayer->SetLocalRotation(Quaternion::ToQuaternion(euler));


			mRemotePlayers[static_cast<UINT32>(data->RemoteP_ID)] = remotePlayer;
			//std::cout << "Process Event : AddAnotherPlayer - " << remotePlayer << std::endl;
		}

		break;
		case NetworkEvent::Game::Enum::Move_RemotePlayer:
		{

			NetworkEvent::Game::Move_RemotePlayer* data = reinterpret_cast<NetworkEvent::Game::Move_RemotePlayer*>(EventData.get());
			if (mRemotePlayers.count(data->RemoteP_ID)) {
				rsptr<GridObject> player = mRemotePlayers[data->RemoteP_ID];
				player->GetComponent<Script_RemotePlayer>()->SetPacketPos(data->RemoteP_Pos);
				//player->SetPosition(data->RemoteP_Pos);
			}
			else {
				LOG_MGR->Cout("Player - ", data->RemoteP_ID, "Not Existed\n");
			}
		}

		break;
		case NetworkEvent::Game::Enum::Remove_RemotePlayer:
		{
			std::cout << "RemoveOtherPlayer \n";

			NetworkEvent::Game::Remove_RemotePlayer* data = reinterpret_cast<NetworkEvent::Game::Remove_RemotePlayer*>(EventData.get());
			mRemotePlayers.unsafe_erase(data->RemoteP_ID);
		}
		break;

		case NetworkEvent::Game::Enum::Extrapolate_RemotePlayer:
		{
			NetworkEvent::Game::Extrapolate_RemotePlayer* data = reinterpret_cast<NetworkEvent::Game::Extrapolate_RemotePlayer*>(EventData.get());
			if (!mRemotePlayers.count(data->RemoteP_ID)) {
				return;
			}

			rsptr<GridObject> player = mRemotePlayers[data->RemoteP_ID];

			ExtData ExtrapolatedData        = {};
			ExtrapolatedData.PingTime       = data->PingTime;
			ExtrapolatedData.TargetPos      = data->ExtPos;
			ExtrapolatedData.TargetRot      = data->ExtRot;
			ExtrapolatedData.MoveDir        = data->ExtMoveDir;
			ExtrapolatedData.MoveState		= data->RemoteP_MoveState;
			ExtrapolatedData.Velocity       = data->RemoteVelocity;

			player->GetComponent<Script_RemotePlayer>()->SetExtrapolatedData(ExtrapolatedData);
		}
		break;

		case NetworkEvent::Game::Enum::ChangeAnim_RemotePlayer:
		{
			NetworkEvent::Game::ChangeAnimation_RemotePlayer* data = reinterpret_cast<NetworkEvent::Game::ChangeAnimation_RemotePlayer*>(EventData.get());
			if (!mRemotePlayers.count(data->RemoteP_ID)) {
				return;
			}

			int RemotePlayer_ID = data->RemoteP_ID;
			int NewAnimIndex    = data->AnimationIndex;

			rsptr<GridObject> player = mRemotePlayers[data->RemoteP_ID];
			player->GetAnimator()->GetController()->SetAnimation(NewAnimIndex);

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

void ClientNetworkManager::SetClientCallback_ChangeAnimation(sptr<GridObject> player)
{
	player->GetAnimator()->GetController()->SetAnimationSendCallback(std::bind(&ClientNetworkManager::ClientCallBack_ChangeAnimation, this, std::placeholders::_1));
}

void ClientNetworkManager::ClientCallBack_ChangeAnimation(int index)
{
	auto pkt = FBS_FACTORY->CPkt_PlayerAnimation(index);
	Send(pkt);

}

long long ClientNetworkManager::GetTimeStamp()
{
	return duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void ClientNetworkManager::Send(SPtr_PacketSendBuf pkt)
{
	mClientNetwork->Broadcast(pkt);
}

sptr<NetworkEvent::Game::Add_RemotePlayer> ClientNetworkManager::CreateEvent_Add_RemotePlayer(GamePlayerInfo info)
{
	sptr<NetworkEvent::Game::Add_RemotePlayer> Event = std::make_shared<NetworkEvent::Game::Add_RemotePlayer>();
	
	Event->type				= NetworkEvent::Game::Enum::Add_RemotePlayer;

	Event->RemoteP_ID        = info.Id;
	Event->RemoteP_Name      = info.Name;
	Event->RemoteP_Pos       = info.Pos;
	Event->RemoteP_Rot       = info.Rot;
	Event->RemoteP_Scale     = info.Sca;
	Event->RemoteP_SpineLook = info.SDir;

	return Event;
}

sptr<NetworkEvent::Game::Remove_RemotePlayer> ClientNetworkManager::CreateEvent_Remove_RemotePlayer(int32_t remID)
{
	sptr<NetworkEvent::Game::Remove_RemotePlayer> Event = std::make_shared<NetworkEvent::Game::Remove_RemotePlayer>();
	
	Event->type = NetworkEvent::Game::Enum::Remove_RemotePlayer;

	Event->RemoteP_ID = remID;

	return Event;
}

sptr<NetworkEvent::Game::Move_RemotePlayer> ClientNetworkManager::CreateEvent_Move_RemotePlayer(int32_t remID, Vec3 remotePos, ExtData::MOVESTATE movestate )
{
	sptr<NetworkEvent::Game::Move_RemotePlayer> Event = std::make_shared<NetworkEvent::Game::Move_RemotePlayer>();

	Event->type = NetworkEvent::Game::Enum::Move_RemotePlayer;

	Event->RemoteP_ID        = remID;
	Event->RemoteP_Pos       = remotePos;
	Event->RemoteP_MoveState = movestate;

	return Event;
}

sptr<NetworkEvent::Game::Extrapolate_RemotePlayer> ClientNetworkManager::CreateEvent_Extrapolate_RemotePlayer(int32_t remID, ExtData extdata)
{
	sptr<NetworkEvent::Game::Extrapolate_RemotePlayer> Event = std::make_shared<NetworkEvent::Game::Extrapolate_RemotePlayer>();

	Event->type = NetworkEvent::Game::Enum::Extrapolate_RemotePlayer;

	Event->RemoteP_ID        = remID;
	Event->PingTime          = extdata.PingTime;
	Event->ExtPos            = extdata.TargetPos;
	Event->ExtRot            = extdata.TargetRot;
	Event->ExtMoveDir        = extdata.MoveDir;
	Event->RemoteP_MoveState = extdata.MoveState;
	Event->RemoteVelocity    = extdata.Velocity;

	return Event;
}

sptr<NetworkEvent::Game::ChangeAnimation_RemotePlayer> ClientNetworkManager::CreateEvent_ChangeAnimation_RemotePlayer(int32_t remID, int AnimIndex)
{
	sptr<NetworkEvent::Game::ChangeAnimation_RemotePlayer> Event = std::make_shared<NetworkEvent::Game::ChangeAnimation_RemotePlayer>();

	Event->type = NetworkEvent::Game::Enum::ChangeAnim_RemotePlayer;
	
	Event->RemoteP_ID     = remID;
	Event->AnimationIndex = AnimIndex;

	return Event;
}

long long ClientNetworkManager::GetCurrentTimeMilliseconds()
{
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
