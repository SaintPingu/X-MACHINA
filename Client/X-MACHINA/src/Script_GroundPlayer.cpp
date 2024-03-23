#include "pch.h"
#include "Script_Player.h"

#include "Scene.h"
#include "Object.h"
#include "InputMgr.h"
#include "Component/Rigidbody.h"
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

	SetSpawn(Vec3(250, 100.f, 300));
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
}


void Script_GroundPlayer::UpdateParams(float v, float h)
{
	UpdateParam(v, mParamV);
	UpdateParam(h, mParamH);
}


void Script_GroundPlayer::ProcessInput()
{
	bool isRun{};
	float v{}, h{};

	DWORD dwDirection{};
	DWORD rotationDir{};
	if (KEY_PRESSED('W')) { dwDirection |= Dir::Front; v += 1; }
	if (KEY_PRESSED('S')) { dwDirection |= Dir::Back; v -= 1; }
	if (KEY_PRESSED('A')) { dwDirection |= Dir::Left; h -= 1; }
	if (KEY_PRESSED('D')) { dwDirection |= Dir::Right; h += 1; }
	if (KEY_PRESSED(VK_LSHIFT) && (v != 0 || h != 0)) { isRun = true; }

	if (dwDirection) {
		base::Move(dwDirection);
	}

	if (mAnimator) {
		const auto& controller = mAnimator->GetController();
		if (controller) {
			UpdateParams(v, h);

			Vec3 velocity = mRigid->GetVelocity();
			if (!isRun) {
				mRigid->SetMaxSpeed(100.f);
				controller->SetValue("Run", false);

				if (Vector3::Length(velocity) > 0.1f) {
					controller->SetValue("Walk", true);
				}
				else {
					controller->SetValue("Walk", false);
				}
			}
			else {
				mRigid->SetMaxSpeed(5.f);
				controller->SetValue("Run", true);
				controller->SetValue("Walk", false);
			}
			controller->SetValue("Vertical", mParamV);
			controller->SetValue("Horizontal", mParamH);
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



void Script_GroundPlayer::UpdateParam(float val, float& param)
{
	constexpr float speed = 4.f;				// 파라미터 전환 속도
	constexpr float oppositeExtraSpeed = 20.f;	// 반대편 이동 시 추가 이동 전환 속도

	int sign = Math::Sign(val);					// sign : 파라미터 이동 방향 = 현재 입력값의 부호
	if (Math::IsZero(val)) {					//		  입력이 없다면 현재 파라미터의 반대 부호
		sign = -Math::Sign(param);
	}
	float before = param;
	param += (speed * sign) * DeltaTime();		// 파라미터값 증감

	if (fabs(param) < 0.01f) {					// 0에 근접하면 0으로 설정
		param = 0.f;
	}
	else if (!Math::IsZero(val) && fabs(param) < 0.5f && (fabs(before) > fabs(param))) {	// 반대편 이동 시
		param += (sign * oppositeExtraSpeed) * DeltaTime();									// 추가 전환 속도 적용
	}

	param = std::clamp(param, -1.f, 1.f);		// -1 ~ 1 사이로 고정
}