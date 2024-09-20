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
			constexpr UINT16 Register = 0;
			constexpr UINT16 Add = 1;				// 새로운 플레이어 생성
			constexpr UINT16 Remove = 2;			// Remote 플레이어 제거 
			constexpr UINT16 Move = 3;				// Remote 플레이어 이동 
			constexpr UINT16 Extrapolate = 4;		// Remote 플레이어 위치 예측 
			constexpr UINT16 UpdateAnimation = 5;	// Remote 플레이어 애니메이션 업데이트 
			constexpr UINT16 AimRotation = 6;		// Remote 플레이어 마우스 커서에 의한 방향 전환 업데이트 
			constexpr UINT16 UpdateWeapon = 7;		// Remote 플레이어 Weapon 업데이트 
			constexpr UINT16 OnShoot = 8;			// Remote 플레이어 OnShoot 업데이트 
			constexpr UINT16 OnSkill = 9;			// Remote 플레이어 OnSkioll 업데이트 
			constexpr UINT16 State = 10;			// Remote 플레이어 State 업데이트 

		}

		namespace MonsterType
		{
			constexpr UINT16 Add = 11;
			constexpr UINT16 Remove = 12;
			constexpr UINT16 Move = 13;
			constexpr UINT16 UpdateHP = 14;
			constexpr UINT16 UpdateState = 15;
			constexpr UINT16 Target = 16;
			constexpr UINT16 Dead = 17;
		}

		namespace BulletType
		{
			constexpr UINT16 OnShoot = 18;
			constexpr UINT16 OnHitEnemy = 19;
			constexpr UINT16 OnCollision = 20;
		}

		namespace PheroType
		{
			constexpr UINT16 GetPhero = 21;
		}

		namespace ContentsType					  
		{									  
			constexpr UINT16 Chat = 22;
		}

		namespace ItemType
		{
			constexpr UINT16 Item_Interact = 23;
			constexpr UINT16 Item_ThrowAway = 24;
		}

		/* EVENT DATA */
		struct EventData {
			UINT16 type = {};
		};


		/// +---------------------------------------------------------------------
		///	EVENT ▶▶▶▶▶ REMOTE PLAYER 
		/// ---------------------------------------------------------------------+
		namespace Event_RemotePlayer // RP
		{
			/// >> ADD 
			struct AddBattlePlayer : public EventData {
				GridObject* battlePlayer = nullptr;
			};

			/// >> ADD 
			struct Add : public EventData {
				uint32_t	Id = {};
				std::string Name = {};

				/* Transform Info */
				Vec3		Pos = {};
				Vec4		Rot = {};
				Vec3		SpineLook = {};
			};

			/// >> MOVE 
			struct Move : public EventData {
				uint32_t			Id = {};
				Vec3				Pos = {};
				ExtData::MOVESTATE	MoveState = {};
			};

			/// >> REMOVE 
			struct Remove : public EventData {
				uint32_t			Id = {};

			};

			/// >> EXTRAPOLATE 
			struct Extrapolate : public EventData {
				uint32_t			Id = {};

				ExtData::MOVESTATE	MoveState = {};

				long long			PingTime = {};
				float				Velocity = {};

				Vec3				ExtMoveDir = {};

				Vec3				ExtPos = {};
				Vec3				ExtRot = {};

				float				animparam_h = {};
				float				animparam_v = {};
			};

			/// >> UPDATE ANIMATION 
			struct UpdateAnimation : public EventData {
				uint32_t	Id = {};

				int32_t		animation_upper_index = {};
				int32_t		animation_lower_index = {};
				float		animation_param_h     = {};
				float		animation_param_v     = {};
			};

			struct UpdateAimRotation : public EventData {
				uint32_t id             = {};
				float	 aim_rotation_y = {};
				float	 spine_angle    = {};
			};
			/// >> Update Weapon
			struct UpdateWeapon : public EventData {
				uint32_t Id = {};
				FBProtocol::ITEM_TYPE weapon_type = {};
			};

			/// >> Update On Shoot 
			struct UpdateOnShoot : public EventData {
				uint32_t	id        = {};
				int			bullet_id = {};
				int			weapon_id = {};
				Vec3		ray       = {};
				Vec3		fire_pos  = {};
			};

			/// >> Update On SKill 
			struct UpdateOnSkill : public EventData {
				uint32_t	id                           = {};
				float		phero_amount                 = {};
				FBProtocol::PLAYER_SKILL_TYPE skill_type = {};
				int mindControl_monster_id               = -1;
			};

			/// >> Update Player State 
			struct UpdateState : public EventData {
				uint32_t id                              = {};
				float hp                                 = {};
				float phero                              = {};
				FBProtocol::PLAYER_STATE_TYPE state_type = FBProtocol::PLAYER_STATE_TYPE::PLAYER_STATE_TYPE_NONE;

			};

		}


		/// +---------------------------------------------------------------------
		///	EVENT ▶▶▶▶▶ MONSTER  
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
				int32_t step;
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

			/// >> UPDATE DEAD  
			struct MonsterDead : public EventData {
				int			monster_id;
				Vec3		monster_dead_point;
				std::string pheros;
			};

		}



		/// +---------------------------------------------------------------------
		///	EVENT ▶▶▶▶▶ BULLET  
		/// ---------------------------------------------------------------------+
		namespace Event_Bullet {
			struct OnHitEnemy : public EventData {
				uint32_t Id;

			};
		}

		namespace Event_Phero {
			struct GetPhero : public EventData {
				uint32_t player_id;
				uint32_t phero_id;
			};
		}

		/// +---------------------------------------------------------------------
		///	EVENT ▶▶▶▶▶ Contents  
		/// ---------------------------------------------------------------------+
		namespace Event_Contents {
			struct Chat : public EventData {
				uint32_t		Id;
				std::string		chat;
			};
		
		}
		/// +---------------------------------------------------------------------
		///	EVENT ▶▶▶▶▶ Contents  
		/// ---------------------------------------------------------------------+
		namespace Event_Item {
			struct Item_Interact : public EventData {
				uint32_t				player_id;
				uint32_t				item_id;
				Vec3					drop_Pos;
				FBProtocol::ITEM_TYPE	item_type;
			};

			struct Item_ThrowAway : public EventData {
				uint32_t				player_id;
				uint32_t				item_id;
				Vec3					drop_Pos;
				FBProtocol::ITEM_TYPE	item_type;
			};

		}
	}
}

