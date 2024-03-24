#include "stdafx.h"
#include "Script_ExplosiveObject.h"

#include "Script_Player.h"


void Script_ExplosiveObject::OnCollisionStay(Object& other)
{
	Explode();
}

void Script_ExplosiveObject::Explode()
{
	mIsExploded = true;

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