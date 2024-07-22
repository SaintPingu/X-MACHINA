#include "stdafx.h"
#include "Script_GroundObject.h"

#include "BattleScene.h"
#include "Object.h"




void Script_GroundObject::Start()
{
	base::Start();
	
	if (isnan(mObject->GetPosition().x)) {
		mObject->SetPosition(0, 0, 0);
	}
}

void Script_GroundObject::LateUpdate()
{
	base::LateUpdate();

	if (isnan(mObject->GetPosition().x)) {
		mObject->SetPosition(0, 0, 0);
	}
	Vec3 pos = mObject->GetPosition();
	float terrainHeight = BattleScene::I->GetTerrainHeight(pos.x, pos.z);
	mObject->SetPositionY(terrainHeight);
}
