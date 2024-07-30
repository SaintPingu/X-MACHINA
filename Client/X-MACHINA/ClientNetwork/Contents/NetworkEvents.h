#pragma once

#include "Script_NetworkRemotePlayer.h"
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
			constexpr UINT16 UpdateWeapon	  = 7; // Remote �÷��̾� Weapon ������Ʈ 
			constexpr UINT16 OnShoot		  = 8; // Remote �÷��̾� OnShoot ������Ʈ 
			constexpr UINT16 OnSkill		  = 9; // Remote �÷��̾� OnSkioll ������Ʈ 
			
		}

		namespace MonsterType
		{
			constexpr UINT16 Add			  = 10;
			constexpr UINT16 Remove			  = 11;
			constexpr UINT16 Move			  = 12;
			constexpr UINT16 UpdateHP		  = 13;
			constexpr UINT16 UpdateState	  = 14;
			constexpr UINT16 Target			  = 15;
		}									  
											  
		namespace BulletType				  
		{									  
			constexpr UINT16 OnShoot		  = 16;
			constexpr UINT16 OnCollision	  = 17;
		}									  
											  
		namespace ContentsType					  
		{									  
			constexpr UINT16 Chat			  = 18;
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
				float	 spine_angle    = {};
			};
			/// >> Update Weapon
			struct UpdateWeapon : public EventData {
				uint32_t Id = {};
				FBProtocol::WEAPON_TYPE weapon_type = {};
			};

			/// >> Update On Shoot 
			struct UpdateOnShoot : public EventData {
				uint32_t	id			= {};
				int			bullet_id	= {};
				int			weapon_id	= {};
				Vec3		ray			= {};
			};

			/// >> Update On SKill 
			struct UpdateOnSkill : public EventData {
				uint32_t	id								= {};
				float		phero_amount					= {};
				FBProtocol::PLAYER_SKILL_TYPE skill_type	= {};
				int mindControl_monster_id					= -1;
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
				float		Angle;
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
				FBProtocol::MONSTER_BT_TYPE state;
			};
			struct UpdateState : public EventData {
				std::vector<MonsterUpdateState> Mons;
			};

			/// >> MONSTER TARGET
			struct MonsterTarget {
				int id;
				int target_monster_id;
				int target_player_id;
			};

			struct MonsterTargetUpdate : public EventData {
				std::vector<MonsterTarget> Mons;
			};

		}



		/// +---------------------------------------------------------------------
		///	EVENT ���������� BULLET  
		/// ---------------------------------------------------------------------+
		namespace Bullet {

		}



		/// +---------------------------------------------------------------------
		///	EVENT ���������� MONSTER  
		/// ---------------------------------------------------------------------+
		namespace Event_Contents {
			struct Chat : public EventData {
				uint32_t		Id;
				std::string		chat;
			};
		
		}
	}
}

