#pragma once

#include "Script_RemotePlayer.h"
#include "GameMonster.h"


#undef max
#include "ClientNetwork/Include/Protocol/FBProtocol_generated.h"
class GridObject;

namespace NetworkEvent
{
	namespace Game
	{
		namespace RemotePlayerType
		{
			constexpr UINT16 Add              = 1; // ���ο� �÷��̾� ����
			constexpr UINT16 Remove           = 2; // Remote �÷��̾� ���� 
			constexpr UINT16 Move             = 3; // Remote �÷��̾� �̵� 
			constexpr UINT16 Extrapolate      = 4; // Remote �÷��̾� ��ġ ���� 
			constexpr UINT16 UpdateAnimation  = 5; // Remote �÷��̾� �ִϸ��̼� ������Ʈ 
			constexpr UINT16 AimRotation      = 6; // Remote �÷��̾� ���콺 Ŀ���� ���� ���� ��ȯ ������Ʈ 
		}

		namespace MonsterType
		{
			constexpr UINT16 Add         = 7;
			constexpr UINT16 Remove      = 8;
			constexpr UINT16 Move        = 9;
			constexpr UINT16 UpdateHP    = 10;
			constexpr UINT16 UpdateState = 11;
		}

		namespace BulletType
		{
			constexpr UINT16 OnShoot	 = 12;
			constexpr UINT16 OnCollision = 13;
		}

		/* EVENT DATA */
		struct EventData {
			UINT16 type = {};
		};

		/// +---------------------------------------------------------------------
		///	EVENT ���������� REMOTE PLAYER 
		/// ---------------------------------------------------------------------+
		namespace Event_RemotePlayer // RP
		{
			/// >> ADD 
			struct Add : public EventData {
				uint32_t	Id		 = {}; 
				std::string Name		 = {};

				/* Transform Info */
				Vec3		Pos       = {};
				Vec4		Rot       = {};
				Vec3		SpineLook = {};
			};

			/// >> MOVE 
			struct Move : public EventData {
				uint32_t			Id        = {};
				Vec3				Pos       = {};
				ExtData::MOVESTATE	MoveState = {};
			};

			/// >> REMOVE 
			struct Remove : public EventData {
				uint32_t			Id = {};

			};

			/// >> EXTRAPOLATE 
			struct Extrapolate : public EventData {
				uint32_t			Id			= {};
				
				ExtData::MOVESTATE	MoveState	= {};

				long long			PingTime		= {};
				float				Velocity		= {};

				Vec3				ExtMoveDir	= {};

				Vec3				ExtPos		= {};
				Vec3				ExtRot		= {};

				float				animparam_h	= {};
				float				animparam_v	= {};
			};

			/// >> UPDATE ANIMATION 
			struct UpdateAnimation : public EventData {
				uint32_t	Id = {};

				int32_t		animation_upper_index = {};
				int32_t		animation_lower_index = {};
				float		animation_param_h = {};
				float		animation_param_v = {};
			};

			struct UpdateAimRotation : public EventData {
				uint32_t id             = {};
				float	 aim_rotation_y = {};
			};
		}


		/// +---------------------------------------------------------------------
		///	EVENT ���������� MONSTER  
		/// ---------------------------------------------------------------------+
		namespace Event_Monster // MON
		{
			/// >> ADD
			struct Add : public EventData {
				std::vector<GameMonsterInfo> NewMonsterInfos;
			};

			/// >> REMOVE 
			struct Remove : public EventData {
				std::vector<uint32_t> IDs;
			};

			/// >> MOVE 
			struct MonsterMove {
				uint32_t	Id;
				Vec3		Pos;
				Vec3		Rot;
			};
			struct Move : public EventData {
				std::vector<MonsterMove> Mons;
			};

			/// >> UPDATE HP 
			struct MonsterHP {
				uint32_t Id;
				float	 HP;
			};
			struct UpdateHP : public EventData {
				std::vector<MonsterHP> Mons;
			};

			/// >> UPDATE STATE 
			struct MonsterUpdateState {
				uint32_t Id;
				FBProtocol::MONSTER_STATE_TYPE state;
			};
			struct UpdateState : public EventData {
				std::vector<MonsterUpdateState> Mons;
			};
		}



		/// +---------------------------------------------------------------------
		///	EVENT ���������� BULLET  
		/// ---------------------------------------------------------------------+
		namespace Bullet {

		}
	}
}

