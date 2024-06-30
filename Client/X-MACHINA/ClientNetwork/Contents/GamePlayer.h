#pragma once
#undef max
#include "ClientNetwork/Include/Protocol/FBProtocol_generated.h"

struct GamePlayerInfo
{
	std::string			   Name = {};
	int32_t			   Id   = {};

	/* Transform Info */
	Vec3 Pos  = {}; // Position
	Vec4 Rot  = {}; // Rotation 
	Vec3 Sca  = {}; // Scale 
	Vec3 SDir = {}; // Spine Look Direction 
};

class GamePlayer
{
private:
	GamePlayerInfo mInfo = {};

public:
	GamePlayer();
	~GamePlayer();

};

