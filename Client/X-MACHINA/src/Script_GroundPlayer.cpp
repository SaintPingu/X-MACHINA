#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Bullet.h"
#include "Script_GroundObject.h"
#include "Script_AimController.h"
#include "Script_Weapon.h"

#include "Component/Rigidbody.h"
#include "Component/Camera.h"

#include "Scene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "InputMgr.h"
#include "Timer.h"
#include "Animator.h"
#include "AnimatorMotion.h"
#include "AnimatorController.h"


#include "Component/UI.h"



const float Script_GroundPlayer::mkSitWalkSpeed   = 1.5f;
const float Script_GroundPlayer::mkStandWalkSpeed = 2.2f;
const float Script_GroundPlayer::mkRunSpeed       = 5.f;
const float Script_GroundPlayer::mkSprintSpeed    = 8.f;

const float Script_GroundPlayer::mkStartRotAngle  = 45.f;




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

	mAimController = mObject->AddComponent<Script_AimController>();
}

void Script_GroundPlayer::Start()
{
	base::Start();

	mPlayerType = PlayerType::Human;
	mRotationSpeed = 360.f;

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

	// keep the muzzle facing the target //
	if (mIsAim && mMuzzle) {

		// angle could be too large if aim is so close
		constexpr float kAimMinDistance = 300.f;
		Vec2 aimScreenPos = mAimController->GetAimPos();
		if (aimScreenPos.Length() < kAimMinDistance) {
			aimScreenPos = Vector2::Normalized(aimScreenPos) * kAimMinDistance;
		}

		float angle = GetAngleToAim(aimScreenPos);
		if (fabs(angle) > 0.1f) {
			mSpineBone->RotateGlobal(Vector3::Up, angle);

			// second correction (maybe screen coordinate system is a little inaccurate) //
			angle = GetAngleToAim(aimScreenPos);
			mSpineBone->RotateGlobal(Vector3::Up, angle);
			angle = GetAngleToAim(aimScreenPos);
			mSpineBone->RotateGlobal(Vector3::Up, angle);

			constexpr float rightAngleCorr = 10.f;	// can rotate more by [rightAngleCorr] angle to the right.
			mSpineDstAngle = Vector3::SignedAngle(mObject->GetLook().xz(), mSpineBone->GetUp().xz(), Vector3::Up) - rightAngleCorr;
			// spine angle is max [mkStartRotAngle] degree from object direction, so can't rotate anymore //
			if (fabs(mSpineDstAngle) > mkStartRotAngle) {
				const int sign = Math::Sign(mSpineDstAngle);
				const float corrAngle = (fabs(mSpineDstAngle) - mkStartRotAngle) * -sign;
				mSpineBone->RotateGlobal(Vector3::Up, corrAngle);
			}

			// [DEBUG CODE] show aim UI of muzzle's destination //
			Vec2 muzzleScreenPos = mainCamera->WorldToScreenPoint(mMuzzle->GetPosition());
			Vec2 muzzleScreenLook = Vector2::Normalized(mainCamera->WorldToScreenPoint(mMuzzle->GetPosition() + mMuzzle->GetLook()) - muzzleScreenPos);
			Vec2 bulletDstScreenPos = muzzleScreenPos + (muzzleScreenLook * (aimScreenPos.Length() - muzzleScreenPos.Length()));
			mAimController->mUI2->SetPosition(bulletDstScreenPos);
		}
	}
}


void Script_GroundPlayer::UpdateParams(Dir dir, float v, float h, float rotAngle)
{
	if (mIsAim) {										// ���� ���¶��, �÷��̾��� look���⿡ ���� �ٸ� �ٸ�(Legs) �ִϸ��̼��� ����Ѵ�.
		//constexpr float 

		const float rotAngleAbs = fabs(rotAngle);
		// �̵� ���� //
		if (!Math::IsZero(v) || !Math::IsZero(h)) {
			const Vec3 movementDir = Transform::GetWorldDirection(dir);

			const float lookAngle = Vector3::SignedAngle(mObject->GetLook().xz(), Vector3::Forward, Vector3::Up);
			Vec3 rotatedMovementDir = Vector3::Normalized(Vector3::Rotate(movementDir, Vector3::Up, lookAngle));

			// v, h�� ����, dir�� ũ�Ⱑ �׻� ���簢�� ������ �굵�� �Ѵ�.
			// BlendTree�� �� ���� ���簢���� ���� ��ġ�� �ֱ� �����̴�.
			// �Ʒ� ����� ���� ������ ����Ȯ�� v, h���� ��������. (���� ���ڿ�������)
			{
				float dirAngle = Vector3::SignedAngle(Vector3::Forward, rotatedMovementDir, Vector3::Up);

				float newAngle  = std::fmod(dirAngle, 45.f);
				float newAngle2 = std::fmod(dirAngle, 90.f);

				if (newAngle2 >= 45.f) {
					newAngle = 45 - newAngle;
				}
				float cosTheta = cos(XMConvertToRadians(newAngle));
				float mag = cosTheta > FLT_EPSILON ? fabs(1.f / cosTheta) : 1.f;
				rotatedMovementDir *= mag;

				v = rotatedMovementDir.z;
				h = rotatedMovementDir.x;
				if (fabs(mParamV) + fabs(mParamH) > 1.f) {
					v = fabs(v) > 0.45f ? 1 * Math::Sign(v) : 0.f;
				}
			}
		}
		// ���� ���¿��� ȸ�� //
		else if (rotAngleAbs > 10.f) {
			mController->SetValue("Walk", true);

			// ȸ�� ��ȣ�� ���� h���� �����Ѵ�.
			h = Math::Sign(rotAngle) > 0 ? -1.f : 1.f;
			// 90���� �� h�� �ִ밪 1�� ����, 90�� �̸��̸� h�� �����Ѵ�.
			if (rotAngleAbs < 90.f) {
				h *= rotAngleAbs / 90.f;
			}
		}
		// ���� ���� //
		else {
			mController->SetValue("Walk", false);
		}
	}
	else {
		if (!Math::IsZero(v) || !Math::IsZero(h)) {		// ���� ���°� �ƴϸ� ������ ������ �̵��Ѵ�.
			v = 1;
		}
		else {
			v = 0;
		}
		h = 0;
	}

	UpdateParam(v, mParamV);
	UpdateParam(h, mParamH);
}


void Script_GroundPlayer::ProcessInput()
{
	base::ProcessInput();

	// Ű �Է¿� ���� ���� ����
	Dir dir{};
	float v{}, h{};
	if (KEY_PRESSED('W')) { v += 1; }
	if (KEY_PRESSED('S')) { v -= 1; }
	if (KEY_PRESSED('A')) { h -= 1; }
	if (KEY_PRESSED('D')) { h += 1; }

	dir |= Math::IsZero(v) ? Dir::None : (v > 0) ? Dir::Front : Dir::Back;
	dir |= Math::IsZero(h) ? Dir::None : (h > 0) ? Dir::Right : Dir::Left;

	// ���� ĳ������ ������ ���¸� Ű �Է¿� ���� �����Ѵ�.
	Movement crntMovement = Movement::None;
	// Stand / Sit
	if (KEY_PRESSED('C')) { crntMovement |= Movement::Sit; }
	else { crntMovement |= Movement::Stand; }
	// Walk / Run / Sprint
	if (dir != Dir::None) {
		if (KEY_PRESSED(VK_SHIFT)) { crntMovement |= Movement::Sprint; }
		else if (KEY_PRESSED(VK_CONTROL)) { crntMovement |= Movement::Walk; }
		else { crntMovement |= Movement::Run; }
	}

	Movement crntState = crntMovement & 0x0F;
	Movement prevState = mPrevMovement & 0x0F;

	Movement crntMotion = crntMovement & 0xF0;
	Movement prevMotion = mPrevMovement & 0xF0;

	// ���� ������ ���¿� �ٸ� ��츸 ���� ������Ʈ �Ѵ�.
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
		else {
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

	float rotAngle = 0.f;

	Move(dir);
	if (!mIsAim) {
		RotateTo(dir);
	}
	else {
		RotateToAim(dir, rotAngle);
	}

	if (mController) {
		UpdateParams(dir, v, h, rotAngle);
		mController->SetValue("Vertical", mParamV);
		mController->SetValue("Horizontal", mParamH);
	}
}



void Script_GroundPlayer::Move(Dir dir)
{
	if (dir == Dir::None) {
		return;
	}

	const Vec3 dirVec = Transform::GetWorldDirection(dir);
	mObject->Translate(dirVec * mMovementSpeed * DeltaTime());
}

void Script_GroundPlayer::RotateTo(Dir dir)
{
	if (dir == Dir::None) {
		return;
	}

	Vec3 dstDir = Vector3::Forward;
	if (dir & Dir::Left) {
		if (dir & Dir::Back) {
			dstDir = Vector3::LB;
		}
		else if (dir & Dir::Front) {
			dstDir = Vector3::LF;
		}
		else {
			dstDir = Vector3::Left;
		}
	}
	else if (dir & Dir::Right) {
		if (dir & Dir::Back) {
			dstDir = Vector3::RB;
		}
		else if (dir & Dir::Front) {
			dstDir = Vector3::RF;
		}
		else {
			dstDir = Vector3::Right;
		}
	}
	else if (dir & Dir::Back) {
		dstDir = Vector3::Backward;
	}

	const float angle = Vector3::SignedAngle(mObject->GetLook().xz(), dstDir, Vector3::Up);
	constexpr float smoothAngleBound = 10.f;
	// smooth rotation if angle over [smoothAngleBound] degree
	if(fabs(angle) > smoothAngleBound) {
		mObject->Rotate(0, Math::Sign(angle) * mRotationSpeed * DeltaTime(), 0);
	}
	else if(fabs(angle) > FLT_EPSILON) {
		mObject->Rotate(0, angle, 0);
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
		bulletScript->Fire(*mMuzzle, GetBulletSpeed(), GetBulletDamage());
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
		if (mWeapon && mController) {
			mController->SetValue("Aim", true);
			mIsAim = true;
		}

		break;
	case WM_RBUTTONUP:
		if (mWeapon && mController) {
			mController->SetValue("Aim", false);
			mIsAim = false;
			if (mIsInBodyRotation) {
				mIsInBodyRotation = false;

				Movement prevMotion = mPrevMovement & 0xF0;
				if (prevMotion == Movement::None) {
					mController->SetValue("Walk", false);
				}
			}
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
		mMuzzle = mWeapon->FindFrame("FirePos");
	}
}



void Script_GroundPlayer::UpdateParam(float val, float& param)
{
	constexpr float kParamSpeed = 4.f;				// �Ķ���� ��ȯ �ӵ�
	constexpr float kOppositeExtraSpeed = 20.f;		// �ݴ��� �̵� �� �߰� �̵� ��ȯ �ӵ�

	int sign = Math::Sign(val);						// sign : �Ķ���� �̵� ���� = ���� �Է°��� ��ȣ
	if (Math::IsZero(val)) {						//		  �Է��� ���ٸ� ���� �Ķ������ �ݴ� ��ȣ
		sign = -Math::Sign(param);
	}
	float before = param;
	param += (kParamSpeed * sign) * DeltaTime();	// �Ķ���Ͱ� ����

	if (fabs(param) < 0.01f) {						// 0�� �����ϸ� 0���� ����
		param = 0.f;
	}
	else if (!Math::IsZero(val) && fabs(param) < 0.5f && (fabs(before) > fabs(param))) {	// �ݴ��� �̵� ��
		param += (sign * kOppositeExtraSpeed) * DeltaTime();								// �߰� ��ȯ �ӵ� ����
	}
	else if (!Math::IsZero(val)) {
		param = std::clamp(param, -fabs(val), fabs(val));
	}

	param = std::clamp(param, -1.f, 1.f);		// -1 ~ 1 ���̷� ����
}

float Script_GroundPlayer::GetAngleToAim(const Vec2& aimScreenPos) const
{
	Vec2 muzzleScreenPos = mainCamera->WorldToScreenPoint(mMuzzle->GetPosition());
	Vec2 muzzleScreenLook = Vector2::Normalized(mainCamera->WorldToScreenPoint(mMuzzle->GetPosition() + mMuzzle->GetLook().xz()) - muzzleScreenPos);

	// muzzleScreenLook�� aimScreenPos ������ �����ؾ� �Ѵ�. 
	// �Ѿ��� �����ؾ��ϴ� �����̴�.
	Vec2 bulletDstScreenPos = muzzleScreenPos + (muzzleScreenLook * (aimScreenPos.Length() - muzzleScreenPos.Length()));

	// screen ��ǥ�� ���� (����)
	constexpr float err = 0.8f;
	return -Vector2::SignedAngle(bulletDstScreenPos - muzzleScreenPos, aimScreenPos - muzzleScreenPos) * err;
}

void Script_GroundPlayer::RotateToAim(Dir dir, float& rotAngle)
{
	constexpr float kStopRotAngle  = 10.f;
	const Vec2 aimDir = mAimController->GetAimDirection();

	bool moving = dir != Dir::None;
	// spine bone's look vector is aim direction (spine bone gonna look at aim from LateUpdate function)
	// get an angle if end the aim animation
	if (!moving && mController->IsEndTransition("Body")) {
		rotAngle = mSpineDstAngle;
	}
	else {
		rotAngle = Vector3::SignedAngle(mObject->GetLook().xz(), Vec3(aimDir.x, 0, aimDir.y), Vector3::Up);
	}

	const float angleAbs = fabs(rotAngle);

	// look at the aim if moving
	if (moving) {
		Rotate(rotAngle);
	}
	// rotate body to the aim if spine bone's angle too large
	else if (mIsInBodyRotation || angleAbs > mkStartRotAngle) {
		if (angleAbs > mkStartRotAngle) {
			mIsInBodyRotation = true;
		}
		else if (angleAbs < kStopRotAngle) {
			mIsInBodyRotation = false;
			rotAngle = 0.f;
			return;
		}

		Rotate(rotAngle);
	}
	else {
		rotAngle = 0.f;
	}
}

void Script_GroundPlayer::Rotate(float angle) const
{
	const int sign = Math::Sign(angle);
	const float angleAbs = fabs(angle);

	// 90���� ������ �ִ� �ӵ�, �� ���ϸ� ����
	float rotationSpeed{};
	if (angleAbs > 90.f) {
		rotationSpeed = mRotationSpeed;
	}
	else {
		rotationSpeed = (angleAbs / 90.f) * mRotationSpeed;	// interpolation for smooth rotation
	}

	mObject->Rotate(0, sign * rotationSpeed * DeltaTime(), 0);
}
