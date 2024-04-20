#pragma once

class GridObject;

namespace NetworkEvent
{
	namespace Scene
	{
		namespace Enum
		{
			constexpr UINT16 AddAnotherPlayer = 1;
			constexpr UINT16 MoveOtherPlayer = 2;
			constexpr UINT16 RemoveOtherPlayer = 3;
			constexpr UINT16 Test = 4;
		}


		struct EventData {
			UINT16 type = {};
		};
		struct AddOtherPlayer : public EventData {
			sptr<GridObject>	player = {};
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

