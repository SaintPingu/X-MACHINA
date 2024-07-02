#pragma once
#undef max
#include "ClientNetwork/Include/Protocol/FBProtocol_generated.h"


enum class MonsterType : UINT8
{
	Ursacetus,
	Onyscidus,
	AdvancedCombatDroid_5

};

struct GameMonsterInfo {

	std::string			   Name = {};
	uint32_t			   Id = {};
	MonsterType			   Type = {};
	/* Transform Info */
	Vec3 Pos = {}; // Position
	Vec4 Rot = {}; // Rotation 
	Vec3 SDir = {}; // Spine Look Direction 
};
class GameMonster
{
private:
	GameMonsterInfo mInfo = {};

public:
	GameMonster();
	~GameMonster();
};

