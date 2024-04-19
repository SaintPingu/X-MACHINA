#include "stdafx.h"
#include "Script_ServerManager.h"

#include "Script_GroundObject.h"
#include "Script_NetworkObject.h"

#include "Object.h"
#include "Scene.h"

#define USE_SERVER

void Script_ServerManager::Update()
{
#ifndef USE_SERVER
	return;
#endif

	ProcessEvents();
}

void Script_ServerManager::OnDestroy()
{
	mObject->RemoveComponent<Script_ServerManager>();
}


void Script_ServerManager::ProcessEvents()
{
	WRITE_LOCK;// 임시 큐 생성하여 두 큐 내용 교환	 // <-- Memory Leak here
	//std::cout << "Process Events Start\n";


	while (!mEventsProcessQueue.empty()) {
		sptr<SceneEvent::EventData> EventData = nullptr;
		//std::cout << "Process Events try_pop start \n";

		mEventsProcessQueue.try_pop(EventData);
		if (EventData == nullptr) continue;
		//std::cout << "Process Events try_pop end - ";
		switch (EventData->type)
		{
		case SceneEvent::Enum::AddAnotherPlayer:
		{
			//std::cout << "AddAnotherPlayer \n";

			SceneEvent::AddOtherPlayer* data = reinterpret_cast<SceneEvent::AddOtherPlayer*>(EventData.get());
			mPlayers[data->player->GetID()] = data->player;
			data->player->AddComponent<Script_NetworkObject_GroundPlayer>();
			data->player->AddComponent<Script_GroundObject>();
			data->player->SetPosition(105, 0, 105);

			//std::cout << "Process Event : AddAnotherPlayer - " << data->player << std::endl;
		}

		break;
		case SceneEvent::Enum::MoveOtherPlayer:
		{
			//std::cout << "MoveOtherPlayer \n";

			SceneEvent::MoveOtherPlayer* data = reinterpret_cast<SceneEvent::MoveOtherPlayer*>(EventData.get());
			rsptr<GridObject> player = mPlayers[data->sessionID];
			if (player) {
				// player->GetComponent<Script_NetworkObject>()->UpdateData(data);	// TODO : real data here
			}
			else {
				//std::cout << "Player - " << data->sessionID << " Not Existed\n";
			}
		}

		break;
		case SceneEvent::Enum::RemoveOtherPlayer:
		{
			//std::cout << "RemoveOtherPlayer \n";

			SceneEvent::RemoveOtherPlayer* data = reinterpret_cast<SceneEvent::RemoveOtherPlayer*>(EventData.get());
			mPlayers.unsafe_erase(data->sessionID);
		}
		break;

		case SceneEvent::Enum::Test:
		{
			SceneEvent::Test* data = reinterpret_cast<SceneEvent::Test*>(EventData.get());
			rsptr<GridObject> player = mPlayers[data->sessionID];
			player->GetComponent<Script_NetworkObject>()->UpdateData((void*)data);
		}
		break;

		}

	}
	//std::cout << "Process Events End\n";


	//while (!mEventsQueue_AddOtherPlayer.empty()) {
	//	sptr<GridObject> otherPlayer = nullptr;
	//	mEventsQueue_AddOtherPlayer.try_pop(otherPlayer);
	//	if (otherPlayer) {
	//		std::cout << " *** ADD Dynamic Object *** \n";
	//		std::cout << otherPlayer->GetID() << "\n";
	//		std::cout << "****************************\n";
	//		AddDynamicObject(otherPlayer);
	//		otherPlayer->OnEnable();
	//	}
	//}
	//while (!mEventsQueue_MoveOtherPlayer.empty()) {
	//	PlayerUpdateInfo updateinfo{};
	//	mEventsQueue_MoveOtherPlayer.try_pop(updateinfo);
	//	if (updateinfo.player) {
	//		std::cout << updateinfo.player->GetID() << " - PTR : " << updateinfo.player << std::endl;
	//		std::cout << "[MOVE BY PACKET] - " << updateinfo.player->GetID() << " -> Scene Update : " << updateinfo.NewPos.x << " " << updateinfo.NewPos.y << " " << updateinfo.NewPos.z << "\n";
	//		updateinfo.player->SetPosition(updateinfo.NewPos);
	//	}
	//}
}


void Script_ServerManager::AddEvent(rsptr<SceneEvent::EventData> data)
{
	WRITE_LOCK;
	mEventsProcessQueue.push(data);
}