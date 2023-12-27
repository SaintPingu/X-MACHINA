#include "stdafx.h"
#include "Script_ExplosiveObject.h"
#include "Scene.h"
#include "Object.h"
#include "Script_Player.h"


void Script_ExplosiveObject::Explode()
{
	mIsExploded = true;

	crntScene->GetPlayer()->GetComponent<Script_AirplanePlayer>()->AddScore(1);

	if (CreateFX) {
		CreateFX(mObject->GetPosition());
	}
}

void Script_ExplosiveObject::Hit(float damage)
{
	mHP -= damage;
	if (mHP <= 0) {
		Explode();
	}
}

void Script_ExplosiveObject::OnCollisionStay(CObject& other)
{
	Explode();
}