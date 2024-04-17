#include "stdafx.h"
#include "Script_GroundObject.h"

#include "Scene.h"
#include "Object.h"




void Script_GroundObject::LateUpdate()
{
	Vec3 pos = mObject->GetPosition();
	float terrainHeight = Scene::I->GetTerrainHeight(pos.x, pos.z);
	mObject->SetPositionY(terrainHeight);
}
