#include "stdafx.h"
#include "ClientNetworkManager.h"

#include "Script_GroundObject.h"
#include "Script_NetworkObject.h"

#include "Object.h"
#include "Scene.h"

#include "IocpLibrary/include/Service.h"
#include "IocpLibrary/include/Session.h"
#include "IocpLibrary/include/SocketUtils.h"
#include "IocpLibrary/include/ThreadManager.h"
#include "FlatBuffers/ServerFBsPktFactory.h"
#include "ServerSession.h"
#include "ThreadManager.h"
#include "PacketFactory.h"
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
	SocketUtils::Init();
	ServerFBsPktFactory::Init();
	/// +--------------------------------------------------------------
	///					        CLIENT NETWORK
	/// --------------------------------------------------------------+
	mClientNetworkService = MakeShared<ClientService>(
													NetAddress(ip, port),
													MakeShared<Iocp>(),
													MakeShared<ServerSession>, // TODO : SessionManager 등
													1);
	mClientNetworkService->Start();
	THREAD_MGR->InitTLS();
}

void ClientNetworkManager::Launch(int ThreadNum)
{
	LOG_MGR->SetColor(TextColor::BrightCyan);
	LOG_MGR->Cout("+--------------------------------------\n");
	LOG_MGR->Cout("       X-MACHINA CLIENT NETWORK        \n");
	LOG_MGR->Cout("--------------------------------------+\n");
	LOG_MGR->SetColor(TextColor::Default);

	/* Server Thread */
	for (int32 i = 0; i < ThreadNum; i++)
	{
		THREAD_MGR->Launch([=]()
			{
				while (true)
				{

					mClientNetworkService->GetIocp()->Dispatch();
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
			mRemotePlayers[data->player->GetID()] = data->player;
			data->player->AddComponent<Script_NetworkObject_GroundPlayer>();
			data->player->AddComponent<Script_GroundObject>();
			data->player->SetPosition(105, 0, 105);

			std::cout << "Process Event : AddAnotherPlayer - " << data->player << std::endl;
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


void ClientNetworkManager::Send_CPkt_KeyInput(GameKeyInfo::KEY		 key
										    , GameKeyInfo::KEY_STATE KeyState
										    , GameKeyInfo::MoveKey	 moveKey
										    , Vec2					 mouseDelta)
{
	auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_KeyInput(key, KeyState, moveKey, mouseDelta);
	mClientNetworkService->Broadcast(CPktBuf);
}

void ClientNetworkManager::Send_CPkt_Transform(Vec3 Pos, Vec3 Rot, Vec3 Scale)
{
	auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_Transform(Pos, Rot, Scale);
	
	mClientNetworkService->Broadcast(CPktBuf);
}
