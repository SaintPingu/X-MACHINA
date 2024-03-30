#include "stdafx.h"
#include "Script_Player.h"
#include "InputMgr.h"

#include "Timer.h"

#include "Component/Rigidbody.h"

#include "Script_Bullet.h"



void Script_ShootingPlayer::Update()
{
	if (mCrntFireDelay >= mMaxFireDelay) {
		if (mIsShooting) {
			mCrntFireDelay = 0.f;
			FireBullet();
		}
	}
	else {
		mCrntFireDelay += DeltaTime();
	}
}

void Script_ShootingPlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID) {
	case WM_LBUTTONDOWN:
		StartFire();
		break;

	case WM_LBUTTONUP:
		StopFire();
		break;

	default:
		break;
	}
}