#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_GroundObject.h"

#include "X-Engine.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"



void Script_Enemy::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_GroundObject>();
	mObject->SetTag(ObjectTag::Enemy);
	mPlayer = engine->GetPlayer();
	mController = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
}