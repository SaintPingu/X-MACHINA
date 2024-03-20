#include "stdafx.h"
#include "Weapon.h"

#include "Object.h"

Weapon::Weapon(rsptr<GridObject> object, Transform* parent)
	:
	mObject(object)
{
	parent->SetChild(object);
}

void Weapon::Enable()
{
	mObject->OnEnable();
}

void Weapon::Disable()
{
	mObject->OnDisable();
}
