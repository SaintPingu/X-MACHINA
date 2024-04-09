#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Bullet.h"
#include "Script_GroundObject.h"

#include "Scene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "InputMgr.h"
#include "Component/Rigidbody.h"
#include "Script_Weapon.h"
#include "Timer.h"
#include "Animator.h"
#include "AnimatorMotion.h"
#include "AnimatorController.h"


static void BulletInitFunc(rsptr<InstObject> bullet)
{
	bullet->SetTag(ObjectTag::Bullet);
	bullet->AddComponent<Script_Bullet>();
	bullet->AddComponent<Rigidbody>();
}

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
		weapon = scene->Instantiate(defaultWeapons.at(weaponType), false);
		if (!weapon) {
			continue;
		}

		weapon->AddComponent<Script_Weapon>();
		Transform* transform = mObject->FindFrame(defaultTransforms.at(weaponType));
		if (!transform) {
			continue;
		}
		transform->SetChild(weapon);
	}

	// Bullet
	mBulletPool = scene->CreateObjectPool("bullet", 200, BulletInitFunc);
	SetFireDelay(0.1f);
	SetBulletSpeed(30.f);
	SetBulletDamage(1.f);

	// others
	mObject->AddComponent<Script_GroundObject>();
	mSpineBone = mObject->FindFrame("Humanoid_ Spine1");
}

void Script_GroundPlayer::Start()
{
	base::Start();

	mPlayerType = PlayerType::Human;
	mRotationSpeed = 60.f;

	SetSpawn(Vec3(100, 0, 100));
	SetHP(150.f);

	mAnimator = mObject->GetObj<GameObject>()->GetAnimator();
	if (mAnimator) {
		mController = mAnimator->GetController();
	}
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

	// lock aim forwards //
	if (mIsAim) {
		constexpr float targetDistance = 10;					// the target is always 10m ahead
		Vec3 bonePos = mSpineBone->GetPosition();
		const Vec3 target(bonePos + mObject->GetLook() * targetDistance);

		Vec3 lookTo = target - bonePos;							// direction of the target
		Vec3 look = mSpineBone->GetUp();						// direction of the bone

		mSpineBone->LookToWorld2(lookTo, look, Vec3::Up);		// rotates bone toward the target

		// keep the muzzle facing the target
		if (mFirePos) {
			// rotates only in the xz-plane
			Vec3 forward = mFirePos->GetLook().xz();
			Vec3 dir = (target - mFirePos->GetPosition()).xz();

			constexpr float err = 0.93f;						// rotation error correction value
			float gunAngle = Vector3::SignedAngle(forward, dir, Vec3::Up) * err;	// angle to target

			mSpineBone->RotateGlobal(Vec3::Up, gunAngle);
		}
	}
}


void Script_GroundPlayer::UpdateParams(float v, float h)
{
	UpdateParam(v, mParamV);
	UpdateParam(h, mParamH);
}


void Script_GroundPlayer::ProcessInput()
{
	base::ProcessInput();

	float v{}, h{};

	Dir dir{};
	Dir rotationDir{};
	if (KEY_PRESSED('W')) { dir |= Dir::Front; v += 1; }
	if (KEY_PRESSED('S')) { dir |= Dir::Back;  v -= 1; }
	if (KEY_PRESSED('A')) { dir |= Dir::Left;  h -= 1; }
	if (KEY_PRESSED('D')) { dir |= Dir::Right; h += 1; }

	// 현재 캐릭터의 움직임 상태를 키 입력에 따라 설정한다.
	Movement crntMovement = Movement::None;
	// Stand / Sit
	if (KEY_PRESSED('C'))				   { crntMovement |= Movement::Sit; }
	else						           { crntMovement |= Movement::Stand; }
	// Walk / Run / Sprint
	if (dir != Dir::None) {
		     if (KEY_PRESSED(VK_SHIFT))    { crntMovement |= Movement::Sprint; }
		else if (KEY_PRESSED(VK_CONTROL))  { crntMovement |= Movement::Walk; }
		else						       { crntMovement |= Movement::Run; }
	}

	Movement crntState = crntMovement & 0x0F;
	Movement prevState = mPrevMovement & 0x0F;

	Movement crntMotion = crntMovement & 0xF0;
	Movement prevMotion = mPrevMovement & 0xF0;

	// 이전 움직임 상태와 다른 경우만 값을 업데이트 한다.
	if (!(crntState & prevState)) {
		switch (prevState) {
		case Movement::None:
		case Movement::Stand:
			break;
		case Movement::Sit:
			mController->SetValue("Sit", false);
			break;

		default:
			assert(0);
			break;
		}

		switch (crntState) {
		case Movement::None:
		case Movement::Stand:
			break;
		case Movement::Sit:
			mController->SetValue("Sit", true);
			break;

		default:
			assert(0);
			break;
		}
	}

	if (!(crntState & prevState) || !(crntMotion & prevMotion)) {
		switch (prevMotion) {
		case Movement::None:
			break;
		case Movement::Walk:
			mController->SetValue("Walk", false);
			break;
		case Movement::Run:
			mController->SetValue("Run", false);
			break;
		case Movement::Sprint:
			mController->SetValue("Sprint", false);
			break;

		default:
			assert(0);
			break;
		}

		switch (crntMotion) {
		case Movement::None:
			break;
		case Movement::Walk:
			mController->SetValue("Walk", true);

			break;
		case Movement::Run:
			if (crntState & Movement::Stand) {
				mController->SetValue("Run", true);
			}
			else {
				mController->SetValue("Walk", true);
				crntMotion = Movement::Walk;
			}

			break;
		case Movement::Sprint:
			if (crntState & Movement::Stand) {
				mController->SetValue("Sprint", true);
			}
			else {
				mController->SetValue("Walk", true);
				crntMotion = Movement::Walk;
			}

			break;
		default:
			assert(0);
			break;
		}
	}

	switch (crntMotion) {
	case Movement::None:
		mMovementSpeed = 0.f;
		break;
	case Movement::Walk:
		if (crntState & Movement::Stand) {
			mMovementSpeed = mkStandWalkSpeed;
		}
		else{
			mMovementSpeed = mkSitWalkSpeed;
		}
		break;
	case Movement::Run:
		mMovementSpeed = mkRunSpeed;
		break;
	case Movement::Sprint:
		mMovementSpeed = mkSprintSpeed;
		break;

	default:
		assert(0);
		break;
	}

	mPrevMovement = crntState | crntMotion;

	if (mController) {
		UpdateParams(v, h);
		mController->SetValue("Vertical", mParamV);
		mController->SetValue("Horizontal", mParamH);
	}

	Move(dir);

	Vec2 mouseDelta = input->GetMouseDelta();
	mPlayer->Rotate(0.f, mouseDelta.x);
}



void Script_GroundPlayer::Move(Dir dir)
{
	if (dir == Dir::None) {
		return;
	}

	const Vec3 dirVec = Vector3::Normalized(mObject->GetDirection(dir));
	mObject->Translate(dirVec * mMovementSpeed * DeltaTime());
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
	if (!mWeapon) {
		return;
	}

	auto& bullet = mBulletPool->Get(true);
	if (bullet) {
		auto& bulletScript = bullet->GetComponent<Script_Bullet>();
		bulletScript->Fire(*mFirePos, GetBulletSpeed(), GetBulletDamage());
	}
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
	base::ProcessMouseMsg(messageID, wParam, lParam);

	switch (messageID) {
	case WM_RBUTTONDOWN:
		if (mAnimator) {
			mAnimator->GetController()->SetValue("Aim", true);
			mIsAim = true;
		}

	break;
	case WM_RBUTTONUP:
		if (mAnimator) {
			mAnimator->GetController()->SetValue("Aim", false);
			mIsAim = false;
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

		default:
			break;
		}
	}

	break;
	case WM_KEYUP:
	{

	}

	break;
	default:
		break;
	}
}



void Script_GroundPlayer::SetWeapon(int weaponIdx)
{
	if (weaponIdx == 0) {
		mAnimator->GetController()->SetValue("Weapon", 0);
		if (mWeapon) {
			mWeapon->OnDisable();
			mWeapon = nullptr;
		}
		return;
	}

	if (mWeapon) {
		if (mWeapon == mWeapons[weaponIdx - 1]) {
			return;
		}
		else {
			mWeapon->OnDisable();
		}
	}

	mWeapon = mWeapons[weaponIdx - 1];
	if (mWeapon) {
		mWeapon->OnEnable();
		mAnimator->GetController()->SetValue("Weapon", weaponIdx);
		mFirePos = mWeapon->FindFrame("FirePos");
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