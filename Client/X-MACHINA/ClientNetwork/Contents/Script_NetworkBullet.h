#pragma once


#include "Script_Network.h"


class Script_NetworkBullet: public Script_Network {
	COMPONENT(Script_NetworkBullet, Script_Network)
private:
	Vec3	mFirePoint;
	Vec3	mFireDir;
	float	mMoveSpeed;





public:
	virtual void Awake() override;
	virtual void Update() override;



};

