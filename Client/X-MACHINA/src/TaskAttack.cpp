#include "stdafx.h"
#include "TaskAttack.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


TaskAttack::TaskAttack(Object* object, float wait, std::function<void()> callback)
	:
	Wait(wait, callback)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}
