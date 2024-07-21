#include "stdafx.h"
#include "Script_GroundObject.h"

#include "BattleScene.h"
#include "Object.h"




void Script_GroundObject::LateUpdate()
{
	Vec3 pos = mObject->GetPosition();
	float terrainHeight = BattleScene::I->GetTerrainHeight(pos.x, pos.z);
	mObject->SetPositionY(terrainHeight);
}
