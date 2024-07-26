#include "stdafx.h"
#include "Script_NetworkShootingRemotePlayer.h"
#include "Object.h"
#include "Script_Item.h"

void Script_NetworkShootingRemotePlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!mWeaponScript) {
		return;
	}

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

void Script_NetworkShootingRemotePlayer::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	base::ProcessKeyboardMsg(messageID, wParam, lParam);

	switch (messageID) {
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 'R':
			if (!IsInGunChangeMotion()) {
				Reload();
			}
			break;

		default:
			break;
		}
	}
	break;
	default:
		break;
	}
}

void Script_NetworkShootingRemotePlayer::StartFire()
{
	mWeaponScript->StartFire();
}

void Script_NetworkShootingRemotePlayer::StopFire()
{
	mWeaponScript->StopFire();
}

bool Script_NetworkShootingRemotePlayer::Reload()
{
	if (mWeaponScript) {
		return mWeaponScript->CheckReload();
	}

	return false;
}

void Script_NetworkShootingRemotePlayer::SetWeapon(int weaponNum)
{
	mCrntWeaponNum = weaponNum;
	mWeapon = mWeapons[weaponNum - 1];
	if (mWeapon) {
		mWeapon->SetActive(true);
		mWeaponScript = mWeapon->GetComponent<Script_Weapon>();
		mMuzzle = mWeaponScript->GetMuzzle();
	}
}

void Script_NetworkShootingRemotePlayer::DrawWeapon(int weaponNum)
{
	if (IsInGunChangeMotion()) {
		return;
	}

	if (weaponNum == 0) {
		if (mWeapon) {
			PutbackWeapon();
		}
		return;
	}

	const int weaponIdx = weaponNum - 1;

	if (mWeapons.size() <= weaponIdx) {
		return;
	}
	if (mWeapons[weaponIdx] == nullptr) {
		return;
	}

	// 이미 무기를 들고 있다면 putback 후 draw한다.
	if (mWeapon) {
		if (mWeapon == mWeapons[weaponIdx]) {
			return;
		}
		else {
			PutbackWeapon();
		}

		mNextWeaponNum = weaponNum;
	}
	else {
		DrawWeaponStart(weaponNum, false);
	}
}

void Script_NetworkShootingRemotePlayer::DrawWeaponStart(int weaponNum, bool isDrawImmed)
{
	if (weaponNum <= 0) {
		return;
	}

	mIsInDraw = true;
	mNextWeaponNum = weaponNum;
}

void Script_NetworkShootingRemotePlayer::DrawWeapon()
{
	if (mNextWeaponNum <= 0 || mNextWeaponNum > mWeapons.size()) {
		mCrntWeaponNum = 0;
		DrawWeaponEnd();
		ResetWeaponAnimation();
		return;
	}

	SetWeapon(mNextWeaponNum);
}

void Script_NetworkShootingRemotePlayer::DrawWeaponEnd()
{
	if (mCrntWeaponNum != mNextWeaponNum) {
		SetWeapon(mNextWeaponNum);
	}
	ResetNextWeaponNum();
	mIsInDraw = false;
}

void Script_NetworkShootingRemotePlayer::PutbackWeapon()
{
	ResetNextWeaponNum();
	mIsInPutback = true;

	if (mWeaponScript) {
		StopFire();
	}
}

void Script_NetworkShootingRemotePlayer::PutbackWeaponEnd()
{
	mIsInPutback = false;

	if (mWeapon) {
		mWeapon->SetActive(false);
		mWeapon = nullptr;
		mWeaponScript = nullptr;
		mMuzzle = nullptr;
	}

	if (mNextWeaponNum != 0) {
		// putback이 끝난 후 바로 draw
		DrawWeaponStart(mNextWeaponNum, true);
	}

	mCrntWeaponNum = 0;
}

void Script_NetworkShootingRemotePlayer::DropWeapon(int weaponIdx)
{
	if (weaponIdx < 0) {
		return;
	}

	auto& weapon = mWeapons[weaponIdx];
	if (weapon) {
		weapon->DetachParent(false);
		weapon->SetLocalRotation(Quat::Identity);
		weapon->SetTag(ObjectTag::Item);

		const auto& weaponItem = weapon->AddComponent<Script_Item_Weapon>();
		weaponItem->StartDrop();
		weapon->SetActive(true);

		if (weapon == mWeapon) {
			mWeapon = nullptr;
			mWeaponScript = nullptr;
			mMuzzle = nullptr;
			mCrntWeaponNum = 0;
		}
		weapon = nullptr;

		mIsInDraw = false;
		mIsInPutback = false;
	}
}