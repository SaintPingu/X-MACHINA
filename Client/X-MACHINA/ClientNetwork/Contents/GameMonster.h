#pragma once
#undef max
#include "ClientNetwork/Include/Protocol/FBProtocol_generated.h"


enum class MonsterType : UINT8
{
	Ursacetus,
	Onyscidus,
	AdvancedCombatDroid_5,
	Anglerox,
	Arack,
	Ceratoferox,
	Gobbler,
	LightBipedMech,
	MiningMech,
	Rapax,
	Aranobot,

	End,
};

struct PheroInfo {
	int ID         = -1;
	int level      = -1;
	int pheroIndex = -1;
};
struct GameMonsterInfo {

	std::string					Name = {};
	int							Id   = {};
	int							Target_Player_Id = -1;
	FBProtocol::MONSTER_TYPE	Type = {};
	FBProtocol::MONSTER_BT_TYPE	bt_type = {};
	/* Transform Info */
	Vec3						Pos  = {}; // Position
	Vec4						Rot  = {}; // Rotation 
	Vec3						SDir = {}; // Spine Look Direction 

	std::vector<PheroInfo>      mPheros{};

	void InitPheros(std::string pheros);

};
class GameMonster
{
private:
	GameMonsterInfo mInfo = {};

public:
	GameMonster();
	~GameMonster();
};

