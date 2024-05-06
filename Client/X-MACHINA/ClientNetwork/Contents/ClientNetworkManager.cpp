#include "stdafx.h"
#include "ClientNetworkManager.h"

#include "Script_GroundObject.h"
#include "Script_NetworkObject.h"

#include "Object.h"
#include "Scene.h"

#include "ClientNetwork/Include/MemoryManager.h"
#include "ClientNetwork/Include/ClientNetwork.h"
#include "ClientNetwork/Contents/ServerSession.h"
#include "ClientNetwork/Include/ThreadManager.h"
#include "ClientNetwork/Include/NetworkManager.h"


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
	/// +--------------------------------------------------------------
	///					        CLIENT NETWORK
	/// --------------------------------------------------------------+
	if (FALSE == NETWORK_MGR->Init()) {
		LOG_MGR->Cout("NETWORK MANAGER INIT FAIL\n");
		return;
	}
	LOG_MGR->Cout("NETWORK MANAGER INIT SUCCESS\n");

	mClientNetwork = Memory::Make_Shared<ClientNetwork>();
	mClientNetwork->SetMaxSessionCnt(1); // 1명 접속  
	mClientNetwork->SetSessionConstructorFunc(std::make_shared<ServerSession>);
	
	if (FALSE == mClientNetwork->Start(L"127.0.0.1", 7777)) {
		LOG_MGR->Cout("CLIENT NETWORK SERVICE START FAIL\n");
		return;
	}
	LOG_MGR->Cout("CLIENT NETWORK SERVICE START SUCCESS\n");
}

void ClientNetworkManager::Launch(int ThreadNum)
{
	
	LOG_MGR->SetColor(TextColor::BrightCyan);
	LOG_MGR->Cout("+--------------------------------------\n");
	LOG_MGR->Cout("       X-MACHINA CLIENT NETWORK        \n");
	LOG_MGR->Cout("--------------------------------------+\n");
	LOG_MGR->SetColor(TextColor::Default);

	for (int i = 0; i < ThreadNum; ++i) {
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
		sptr<NetworkEvent::Scene::EventData> EventData = nullptr;

		mSceneEvnetQueue[FrontIdx].EventsQueue.try_pop(EventData);
		if (EventData == nullptr) continue;
		LOG_MGR->Cout("EVENT TYPE : (1-Add) (2-Mov) (3-Rem)", EventData->type, "\n");
		
		switch (EventData->type)
		{
		case NetworkEvent::Scene::Enum::AddAnotherPlayer:
		{

			NetworkEvent::Scene::AddOtherPlayer* data = reinterpret_cast<NetworkEvent::Scene::AddOtherPlayer*>(EventData.get());

			sptr<GridObject> remotePlayer = Scene::I->Instantiate("EliteTrooper");
			remotePlayer->SetName(data->RemoteP_Name);
			remotePlayer->SetID(data->RemoteP_ID);

			remotePlayer->AddComponent<Script_NetworkObject_GroundPlayer>();
			remotePlayer->AddComponent<Script_GroundObject>();
			
			remotePlayer->SetPosition(data->RemoteP_Pos.x, data->RemoteP_Pos.y, data->RemoteP_Pos.z);
			//Vec4 rot   = remotePlayer->GetRotation();
			//Vec3 euler = Quaternion::ToEuler(rot);
			//euler.y    = data->RemoteP_Rot.y;
			//remotePlayer->SetLocalRotation(Quaternion::ToQuaternion(euler));


			mRemotePlayers[static_cast<UINT32>(data->RemoteP_ID)] = remotePlayer;
			std::cout << "Process Event : AddAnotherPlayer - " << remotePlayer << std::endl;
		}

		break;
		case NetworkEvent::Scene::Enum::MoveOtherPlayer:
		{

			NetworkEvent::Scene::MoveOtherPlayer* data = reinterpret_cast<NetworkEvent::Scene::MoveOtherPlayer*>(EventData.get());
			rsptr<GridObject> player = mRemotePlayers[data->sessionID];
			if (player) {
				std::cout << data->Pos.x << " " << data->Pos.y << " " << data->Pos.z << std::endl;
				player->SetPosition(data->Pos);
			}
			else {
				LOG_MGR->Cout("Player - ", data->sessionID, "Not Existed\n");
			}
		}

		break;
		case NetworkEvent::Scene::Enum::RemoveOtherPlayer:
		{
			std::cout << "RemoveOtherPlayer \n";

			NetworkEvent::Scene::RemoveOtherPlayer* data = reinterpret_cast<NetworkEvent::Scene::RemoveOtherPlayer*>(EventData.get());
			mRemotePlayers.unsafe_erase(data->sessionID);
		}
		break;

		case NetworkEvent::Scene::Enum::Test:
		{
			NetworkEvent::Scene::Test* data = reinterpret_cast<NetworkEvent::Scene::Test*>(EventData.get());
			rsptr<GridObject> player = mRemotePlayers[data->sessionID];
			player->GetComponent<Script_NetworkObject>()->UpdateData((void*)data);
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

void ClientNetworkManager::RegisterEvent(sptr<NetworkEvent::Scene::EventData> data)
{
	mSceneEvnetQueue[mBackSceneEventIndex.load()].EventsQueue.push(data);
}

long long ClientNetworkManager::GetTimeStamp()
{
	return duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}


void ClientNetworkManager::Send_CPkt_KeyInput(GameKeyInfo::KEY		 key
										    , GameKeyInfo::KEY_STATE KeyState
										    , GameKeyInfo::MoveKey	 moveKey
										    , Vec2					 mouseDelta)
{
	//auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_KeyInput(key, KeyState, moveKey, mouseDelta);
	//mClientNetworkService->Broadcast(CPktBuf);
}

void ClientNetworkManager::Send_CPkt_Transform(Vec3 Pos, Vec3 Rot, Vec3 Scale, Vec3 SpineLookDir, long long timestamp)
{
	//auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Transform(Pos, Rot, Scale, SpineLookDir, timestamp);
	
	//mClientNetworkService->Broadcast(CPktBuf);
}
