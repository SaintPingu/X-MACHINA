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

	mObject->SetPositionY(0.f);
}