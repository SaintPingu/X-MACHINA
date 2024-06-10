#include "stdafx.h"
#include "Script_Phero.h"

#include "Timer.h"
#include "Object.h"


void Script_Phero::Start()
{
	mLifeTime = Math::RandFloat(5.f, 7.f);
	
	GameObject* object = dynamic_cast<GameObject*>(mObject);
	if (nullptr != object) {
		object->SetUseShadow(false);
	}
}

void Script_Phero::Update()
{
	mCurrTime += DeltaTime();

	mObject->Rotate(mObject->GetUp(), 1.f);
}

void Script_Phero::LateUpdate()
{
	if (mCurrTime >= mLifeTime) {
		mObject->Destroy();
	}
}
