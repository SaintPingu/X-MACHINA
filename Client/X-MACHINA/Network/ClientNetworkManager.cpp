#include "stdafx.h"
#include "ClientNetworkManager.h"

#include "Script_GroundObject.h"
#include "Script_NetworkObject.h"

#include "Object.h"
#include "Scene.h"


DEFINE_SINGLETON(ClientNetworkManager);
ClientNetworkManager::ClientNetworkManager()
{
}

ClientNetworkManager::~ClientNetworkManager()
{
}

void ClientNetworkManager::ProcessEvents()
{
	SwapEventsQueue();
	int FrontIdx = mFrontSceneEventIndex.load();
	std::cout << "FRONT : " << FrontIdx << " - Q SIZE : " << mSceneEvnetQueue[FrontIdx].EventsQueue.unsafe_size() << std::endl;

	while (!mSceneEvnetQueue[FrontIdx].EventsQueue.empty()) {
		sptr<NetworkEvent::Scene::EventData> EventData = nullptr;

		mSceneEvnetQueue[FrontIdx].EventsQueue.try_pop(EventData);
		if (EventData == nullptr) continue;
		std::cout << "EVENT TYPE : (1-Add) (2-Mov) (3-Rem)" << EventData->type << " \n";
		switch (EventData->type)
		{
		case NetworkEvent::Scene::Enum::AddAnotherPlayer:
		{
			std::cout << "AddAnotherPlayer \n";

			NetworkEvent::Scene::AddOtherPlayer* data = reinterpret_cast<NetworkEvent::Scene::AddOtherPlayer*>(EventData.get());
			mRemotePlayers[data->player->GetID()] = data->player;
			data->player->AddComponent<Script_NetworkObject_GroundPlayer>();
			data->player->AddComponent<Script_GroundObject>();
			data->player->SetPosition(105 + rand() % 10, 0, 105 + rand() % 10);

			std::cout << "Process Event : AddAnotherPlayer - " << data->player << std::endl;
		}

		break;
		case NetworkEvent::Scene::Enum::MoveOtherPlayer:
		{
			std::cout << "MoveOtherPlayer \n";

			NetworkEvent::Scene::MoveOtherPlayer* data = reinterpret_cast<NetworkEvent::Scene::MoveOtherPlayer*>(EventData.get());
			rsptr<GridObject> player = mRemotePlayers[data->sessionID];
			if (player) {
				std::cout << data->Pos.x << " " << data->Pos.y << " " << data->Pos.z << std::endl;
				player->SetPosition(data->Pos);
				//player->GetComponent<Script_NetworkObject>()->UpdateData(data);	// TODO : real data here
			}
			else {
				std::cout << "Player - " << data->sessionID << " Not Existed\n";
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
