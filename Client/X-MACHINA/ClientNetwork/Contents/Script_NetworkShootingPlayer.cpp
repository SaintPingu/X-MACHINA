#include "stdafx.h"
#include "Script_NetworkShootingPlayer.h"
#include "Object.h"
#include "Script_Item.h"
#include "FBsPacketFactory.h"
void Script_NetworkShootingPlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!mWeaponScript) {
		return;
	}

	switch (messageID) {
	case WM_LBUTTONDOWN:

		break;

	case WM_LBUTTONUP:

		break;

	default:
		break;
	}
}

bool Script_NetworkShootingPlayer::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
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

	return true;
}

void Script_NetworkShootingPlayer::StartFire()
{
	mWeaponScript->StartFire();
}

void Script_NetworkShootingPlayer::StopFire()
{
	mWeaponScript->StopFire();
}

bool Script_NetworkShootingPlayer::Reload()
{
	if (mWeaponScript) {
		return mWeaponScript->CheckReload();
	}

	return false;
}

void Script_NetworkShootingPlayer::SetWeapon(int weaponNum)
{
	mCrntWeaponNum = weaponNum;
	mWeapon = mWeapons[weaponNum - 1];
	if (mWeapon) {
		mWeapon->SetActive(true);
		mWeaponScript = mWeapon->GetComponent<Script_Weapon>();
		mMuzzle = mWeaponScript->GetMuzzle();
	}
}

void Script_NetworkShootingPlayer::DrawWeapon(int weaponNum)
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

void Script_NetworkShootingPlayer::DrawWeaponStart(int weaponNum, bool isDrawImmed)
{
	if (weaponNum <= 0) {
		return;
	}

	mIsInDraw = true;
	mNextWeaponNum = weaponNum;
}

void Script_NetworkShootingPlayer::DrawWeapon()
{
	if (mNextWeaponNum <= 0 || mNextWeaponNum > mWeapons.size()) {
		mCrntWeaponNum = 0;
		DrawWeaponEnd();
		ResetWeaponAnimation();
		return;
	}

	SetWeapon(mNextWeaponNum);
}

void Script_NetworkShootingPlayer::DrawWeaponEnd()
{
	if (mCrntWeaponNum != mNextWeaponNum) {
		SetWeapon(mNextWeaponNum);
	}
	ResetNextWeaponNum();
	mIsInDraw = false;
}

void Script_NetworkShootingPlayer::PutbackWeapon()
{
	ResetNextWeaponNum();
	mIsInPutback = true;

	if (mWeaponScript) {
		StopFire();
	}
}

void Script_NetworkShootingPlayer::PutbackWeaponEnd()
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

void Script_NetworkShootingPlayer::DropWeapon(int weaponIdx)
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