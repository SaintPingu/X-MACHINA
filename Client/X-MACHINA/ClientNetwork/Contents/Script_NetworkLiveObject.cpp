#include "stdafx.h"
#include "Script_NetworkLiveObject.h"

void Script_NetworkLiveObject::Awake()
{
	base::Awake();
}

void Script_NetworkLiveObject::Start()
{
	base::Start();

	mCrntHP = mMaxHP;
	mPrevHP = mMaxHP;
}

void Script_NetworkLiveObject::Update()
{
	base::Update();
}

bool Script_NetworkLiveObject::Hit(float damage, Object* instigator)
{
	if (mCrntHP <= 0) {
		return false;
	}

	if (mShieldAmount > 0) {
		mShieldAmount -= damage;
	}
	else {
		mCrntHP -= damage;
	}

	if (mCrntHP <= 0) {
		Dead();
		return true;
	}

	return false;
}

void Script_NetworkLiveObject::Dead()
{
	mIsDead = true;

}

void Script_NetworkLiveObject::Resurrect()
{
	mCrntHP = mMaxHP;
	mIsDead = false;

}
