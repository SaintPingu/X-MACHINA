#include "stdafx.h"
#include "Script_Player.h"

#include "Scene.h"
#include "Object.h"
#include "InputMgr.h"
#include "Rigidbody.h"
#include "Weapon.h"
#include "Timer.h"

#include "Animator.h"
#include "AnimatorController.h"



void Script_GroundPlayer::Awake()
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

	// TODO : Weapon (temp)
	mWeapons.resize(gkWeaponTypeCnt, nullptr);
	for (size_t i = 0; i < gkWeaponTypeCnt; ++i) {
		auto& weapon = mWeapons[i];
		WeaponType weaponType = static_cast<WeaponType>(i);
		const auto& weaponObject = scene->Instantiate(defaultWeapons.at(weaponType), false);
		if (!weaponObject) {
			continue;
		}
		Transform* transform = mObject->FindFrame(defaultTransforms.at(weaponType));
		if (!transform) {
			continue;
		}

		weapon = std::make_shared<Weapon>(weaponObject, transform);
	}
}

void Script_GroundPlayer::Start()
{
	base::Start();

	mPlayerType = PlayerType::Human;
	mRotationSpeed = 60.f;

	SetSpawn(Vec3(300, 0, 300));
	SetHP(150.f);

	mRigid->SetMass(100.f);
	mRigid->SetFriction(30.f);
	mRigid->SetAcc(5000.f);
	mRigid->SetMaxSpeed(1.5f);

	mAnimator = mObject->GetObj<GameObject>()->GetAnimator();
}


void Script_GroundPlayer::Update()
{
	base::Update();

	base::ProcessInput();
	ProcessInput();
}


void Script_GroundPlayer::LateUpdate()
{
	base::LateUpdate();

	Vec3 pos = mObject->GetPosition();
	float terrainHeight = scene->GetTerrainHeight(pos.x, pos.z);
	mObject->SetPositionY(terrainHeight);

	if (mAnimator) {
		mAnimator->LookAt(Vec3(300, 32, 300));
	}
}


void Script_GroundPlayer::UpdateParams()
{
	constexpr float speed = 4.f;
	constexpr float oppositeExtraSpeed = 10.f;

	int signV = Math::Sign(v);
	if (Math::IsZero(v)) {
		signV = -Math::Sign(paramV);
	}
	float beforeV = paramV;

	paramV += (speed * signV) * DeltaTime();
	if (fabs(paramV) < 0.01f) {
		paramV = 0.f;
	}
	else if (!Math::IsZero(v) && fabs(paramV) < 0.5f && (fabs(beforeV) > fabs(paramV))) {
		paramV += (signV * speed) * DeltaTime() * oppositeExtraSpeed;
	}
	paramV = std::clamp(paramV, -1.f, 1.f);

	int signH = Math::Sign(h);
	if (Math::IsZero(h)) {
		signH = -Math::Sign(paramH);
	}
	float beforeH = paramH;
	paramH += (signH * speed) * DeltaTime();

	if (fabs(paramH) < 0.01f) {
		paramH = 0.f;
	}
	else if (!Math::IsZero(h) && fabs(paramH) < 0.5f && (fabs(beforeH) > fabs(paramH))) {
		paramH += (signH * speed) * DeltaTime() * oppositeExtraSpeed;
	}
	paramH = std::clamp(paramH, -1.f, 1.f);
}


void Script_GroundPlayer::ProcessInput()
{
	v = 0, h = 0;

	DWORD dwDirection = 0;
	DWORD rotationDir = 0;
	if (KEY_PRESSED('W')) { dwDirection |= Dir::Front; v += 1; }
	if (KEY_PRESSED('S')) { dwDirection |= Dir::Back; v -= 1; }
	if (KEY_PRESSED('A')) { dwDirection |= Dir::Left; h -= 1; }
	if (KEY_PRESSED('D')) { dwDirection |= Dir::Right; h += 1; }
	if (dwDirection) {
		base::Move(dwDirection);
	}

	if (mAnimator) {
		Vec3 velocity = mRigid->GetVelocity();
		const auto& controller = mAnimator->GetController();
		if (controller) {
			UpdateParams();
			if (Vector3::Length(velocity) > 0.1f) {
				controller->SetValue("Walk", true);
			}
			else {
				controller->SetValue("Walk", false);
			}
			controller->SetValue("Vertical", paramV);
			controller->SetValue("Horizontal", paramH);
		}
	}


	if (KEY_PRESSED(VK_LBUTTON)) {
		Vec2 mouseDelta = InputMgr::Inst()->GetMouseDelta();
		mPlayer->Rotate(0.f, mouseDelta.x);
	}
}



void Script_GroundPlayer::Rotate(DWORD rotationDir, float angle)
{
	angle *= mRotationSpeed;
	if (rotationDir) {
		float zRotation = 0.f;
		if (rotationDir & Dir::Left) {
			zRotation += angle;
		}
		if (rotationDir & Dir::Right) {
			zRotation -= angle;
		}

		if (zRotation != 0.f) {
			mObject->Rotate(0.f, 0.f, zRotation);
		}
	}
}


void Script_GroundPlayer::FireBullet()
{
	// TODO : FireBullet
}


void Script_GroundPlayer::OnCollisionStay(Object& other)
{
	switch (other.GetTag())
	{
	case ObjectTag::Building:
		Explode();
		break;
	default:
		break;
	}
}

void Script_GroundPlayer::SetWeapon(int weaponIdx)
{
	rsptr<Animator> animator = mObject->GetObj<GameObject>()->GetAnimator();

	if (weaponIdx == 0) {
		animator->GetController()->SetValue("Weapon", 0);
		if (mWeapon) {
			mWeapon->Disable();
			mWeapon = nullptr;
		}
		return;
	}

	if (mWeapon) {
		if (mWeapon == mWeapons[weaponIdx - 1]) {
			return;
		}
		else {
			mWeapon->Disable();
		}
	}
	
	mWeapon = mWeapons[weaponIdx - 1];
	if (mWeapon) {
		mWeapon->Enable();
		animator->GetController()->SetValue("Weapon", weaponIdx);
	}
}

void Script_GroundPlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID) {
	case WM_RBUTTONDOWN:
		if (mAnimator) {
			mAnimator->GetController()->SetValue("Aim", true);
		}

	break;
	case WM_RBUTTONUP:
		if (mAnimator) {
			mAnimator->GetController()->SetValue("Aim", false);
		}

	break;
	default:
		break;
	}
}

void Script_GroundPlayer::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID) {
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
			SetWeapon(static_cast<int>(wParam - '0'));
			break;

		case VK_CONTROL:
			if (mAnimator) {
				mAnimator->GetController()->SetValue("Sit", true);
			}

		break;
		default:
			break;
		}
	}

	break;
	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_CONTROL:
			if (mAnimator) {
				mAnimator->GetController()->SetValue("Sit", false);
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