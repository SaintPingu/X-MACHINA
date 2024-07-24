#pragma once
#undef max
#include "ClientNetwork/Include/Protocol/FBProtocol_generated.h"


struct PheroInfo {
	int ID         = -1;
	int level      = -1;
	int pheroIndex = -1;
};
struct GameMonsterInfo {

	std::string					Name = {};
	uint32_t					Id   = {};
	FBProtocol::MONSTER_TYPE	Type = {};
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

