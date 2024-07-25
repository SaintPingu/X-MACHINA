#pragma once

#include "Script_Network.h"

class Script_EnemyNetwork : public Script_Network {
	COMPONENT(Script_EnemyNetwork, Script_Network)
private:


public:
	virtual void Awake() override;
	virtual void LateUpdate() override;
	virtual void UpdateData(const void* data) override;

public:
	void SetPostion(const Vec3& pos);
	void SetRotation(const Vec3& rot);
};

