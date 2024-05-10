#pragma once

class GridObject;

namespace NetworkEvent
{
	namespace Game
	{
		namespace Enum
		{
			constexpr UINT16 Add_RemotePlayer         = 1;
			constexpr UINT16 Move_RemotePlayer        = 2;
			constexpr UINT16 Remove_RemotePlayer      = 3;
			constexpr UINT16 Extrapolate_RemotePlayer = 4;

		}

		/// +-------------------------------------------
		///					EVENT DATA 
		/// -------------------------------------------+
		struct EventData {
			UINT16 type = {};
		};

		/// +-------------------------------------------
		///				ADD REMOTE PLAYER 
		/// -------------------------------------------+
		struct Add_RemotePlayer : public EventData {
			
			/* Remote Player Info */
			uint64_t	RemoteP_ID			= {};
			std::string RemoteP_Name		= {};
			/* Transform Info */
			Vec3		RemoteP_Pos			= {};
			Vec3		RemoteP_Rot			= {};
			Vec3		RemoteP_Scale		= {};
			Vec3		RemoteP_SpineLook	= {};
		};


		/// +-------------------------------------------
		///				MOVE REMOTE PLAYER  
		/// -------------------------------------------+

		struct Move_RemotePlayer : public EventData {
			UINT16	RemoteP_ID  = {};
			Vec3	RemoteP_Pos = {};
		};

		/// +-------------------------------------------
		///				REMOVE REMOTE PLAYER 
		/// -------------------------------------------+

		struct Remove_RemotePlayer : public EventData {
			UINT16 RemoteP_ID = {};
		};

		/// +-------------------------------------------
		///				TEST
		/// -------------------------------------------+

		struct Test : public EventData {
			UINT16	sessionID;
			Vec3	Pos{};
			float	Angle{};
			float	SpineAngle{};

			PlayerMotion PlayerMotion{};
			int WeaponID{};
		};

		/// +-------------------------------------------
		///			 EXTRAPOLATE REMOTE PLAYER 
		/// -------------------------------------------+

		struct Extrapolate_RemotePlayer : public EventData {
			long long		PingTime       = {};
			float			RemoteVelocity = {};
			UINT16			RemoteP_ID     = {};
			Vec3			ExtPos         = {};
			Vec3			ExtRot         = {};
			Vec3			ExtMoveDir     = {};
		};

	}
}

