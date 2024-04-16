#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_LiveObject.h"
#include "Timer.h"



void Script_LiveObject::Awake()
{
}

void Script_LiveObject::Start()
{
	mCrntHP = mMaxHP;
}

void Script_LiveObject::Update()
{
}

void Script_LiveObject::Hit(float damage)
{
	mCrntHP -= damage;
	if (mCrntHP <= 0) {
		mIsDead = true;
	}
}

void Script_LiveObject::Dead()
{
	mObject->OnDestroy();
}

void Script_LiveObject::Resurrect()
{
	mCrntHP = mMaxHP;
	mIsDead = false;
}
