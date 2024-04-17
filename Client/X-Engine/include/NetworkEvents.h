#pragma once

class GridObject;

namespace SceneEvent
{
	namespace Enum 
	{
		constexpr UINT16 AddAnotherPlayer  = 1;
		constexpr UINT16 MoveOtherPlayer   = 2;
		constexpr UINT16 RemoveOtherPlayer = 3;
	}


	struct EventData {
		UINT16 type = {};
	};
	struct AddOtherPlayer : public EventData {
		sptr<GridObject>	player			 = {};
	};
	struct MoveOtherPlayer : public EventData {
		UINT16				sessionID        = {};
		Vec3				Pos              = {};
	};
	struct RemoveOtherPlayer : public EventData {
		UINT16 sessionID;
	};

}



class NetworkEvents
{


};

