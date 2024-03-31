#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_LiveObject.h"



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
		Dead();
	}
}

void Script_LiveObject::Dead()
{
	mObject->OnDestroy();
}
