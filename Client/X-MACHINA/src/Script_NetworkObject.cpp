#include "stdafx.h"
#include "Script_NetworkObject.h"
#include "Script_Weapon.h"

#include "Scene.h"
#include "Object.h"

#include "ClientNetwork/Contents/NetworkEvents.h"


void Script_NetworkObject_GroundPlayer::Awake()
{
	base::Awake();

	mSpineBone = mObject->FindFrame("Humanoid_ Spine1");
	InitWeapons();
}

void Script_NetworkObject_GroundPlayer::LateUpdate()
{
	base::LateUpdate();

	mSpineBone->RotateGlobal(Vector3::Up, mSpineAngle);
}

void Script_NetworkObject_GroundPlayer::UpdateData(const void* data)
{
	switch (0) { // data type here
	case 0:
	{
		//const Data_Move_GroundPlayer* moveData = (const Data_Move_GroundPlayer*)data;
		const NetworkEvent::Game::Test* moveData = (const NetworkEvent::Game::Test*)data;
		mObject->SetPosition(moveData->Pos);
		Vec4 rotation = Quaternion::ToQuaternion(Vec3(0, moveData->Angle, 0));
		mObject->SetLocalRotation(rotation);
		mSpineAngle = moveData->SpineAngle;
	}
		break;
	}
}

void Script_NetworkObject_GroundPlayer::InitWeapons()
{
	const std::unordered_map<WeaponType, std::string> defaultWeapons{
		{WeaponType::HandedGun, "SM_SciFiLaserGun" },
		{WeaponType::AssaultRifle, "SM_SciFiAssaultRifle_01" },
		{WeaponType::LightingGun, "SM_SciFiLightingGun" },
		{WeaponType::GatlinGun, "SM_SciFiLaserGatlinGun" },
		{WeaponType::ShotGun, "SM_SciFiShotgun" },
		{WeaponType::MissileLauncher, "SM_SciFiMissileLauncher" },
	};

	const std::unordered_map<WeaponType, std::string> defaultTransforms{
		{WeaponType::HandedGun, "RefPos2HandedGun_Action" },
		{WeaponType::AssaultRifle, "RefPosAssaultRifle_Action" },
		{WeaponType::LightingGun, "RefPosLightningGun_Action" },
		{WeaponType::GatlinGun, "RefPosLaserGatlinGun_Action" },
		{WeaponType::ShotGun, "RefPosShotgun_Action" },
		{WeaponType::MissileLauncher, "RefPosMissileLauncher_Action" },
	};

	// weapon °´Ã¼ »ý¼º //
	mWeapons.resize(gkWeaponTypeCnt, nullptr);
	for (size_t i = 0; i < gkWeaponTypeCnt; ++i) {
		auto& weapon = mWeapons[i];
		WeaponType weaponType = static_cast<WeaponType>(i);
		weapon = Scene::I->Instantiate(defaultWeapons.at(weaponType), false);
	}
}