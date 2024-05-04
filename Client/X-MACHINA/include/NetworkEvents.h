#pragma once

class GridObject;

namespace NetworkEvent
{
	namespace Scene
	{
		namespace Enum
		{
			constexpr UINT16 AddAnotherPlayer  = 1;
			constexpr UINT16 MoveOtherPlayer   = 2;
			constexpr UINT16 RemoveOtherPlayer = 3;
			constexpr UINT16 Test              = 4;
		}


		struct EventData {
			UINT16 type = {};
		};
		struct AddOtherPlayer : public EventData {
			
			/* Remote Player Info */
			uint64_t	RemoteP_ID{};
			std::string RemoteP_Name{};
			Vec3		RemoteP_Pos{};
			Vec3		RemoteP_Rot{};
			Vec3		RemoteP_Scale{};
			Vec3		RemoteP_SpineLook{};

		};
		struct MoveOtherPlayer : public EventData {
			UINT16				sessionID = {};
			Vec3				Pos = {};
		};
		struct RemoveOtherPlayer : public EventData {
			UINT16 sessionID;
		};

		struct Test : public EventData {
			UINT16 sessionID;
			Vec3 Pos{};
			float Angle{};
			float SpineAngle{};

			PlayerMotion PlayerMotion{};
			int WeaponID{};
		};
	}
}



class NetworkEvents
{


};

