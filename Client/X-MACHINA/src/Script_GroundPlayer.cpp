#include "stdafx.h"
#include "Script_Player.h"

#include "Scene.h"
#include "Object.h"
#include "InputMgr.h"
#include "Component/Rigidbody.h"
#include "Script_Weapon.h"
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
	float v{}, h{};

	DWORD dwDirection{};
	DWORD rotationDir{};
	if (KEY_PRESSED('W')) { dwDirection |= Dir::Front; v += 1; }
	if (KEY_PRESSED('S')) { dwDirection |= Dir::Back;  v -= 1; }
	if (KEY_PRESSED('A')) { dwDirection |= Dir::Left;  h -= 1; }
	if (KEY_PRESSED('D')) { dwDirection |= Dir::Right; h += 1; }

	// 현재 캐릭터의 움직임 상태를 키 입력에 따라 설정한다.
	Movement crntMovement = Movement::None;
	// Stand / Sit
	if (KEY_PRESSED('C'))				    { crntMovement |= Movement::Sit; }
	else						            { crntMovement |= Movement::Stand; }
	// Walk / Run / Sprint
	if (dwDirection) {
		if (KEY_PRESSED(VK_LSHIFT))         { crntMovement |= Movement::Sprint; }
		else if (KEY_PRESSED(VK_LCONTROL))  { crntMovement |= Movement::Walk; }
		else						        { crntMovement |= Movement::Run; }
	}

	Movement crntState = static_cast<Movement>(crntMovement & 0x0F);
	Movement prevState = static_cast<Movement>(mPrevMovement & 0x0F);

	Movement crntMotion = static_cast<Movement>(crntMovement & 0xF0);
	Movement prevMotion = static_cast<Movement>(mPrevMovement & 0xF0);

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

	if (!(crntMotion & prevMotion)) {
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

	mPrevMovement = static_cast<Movement>(crntState | crntMotion);

	if (mController) {
		UpdateParams(v, h);
		mController->SetValue("Vertical", mParamV);
		mController->SetValue("Horizontal", mParamH);
	}

	Move(dwDirection);

	if (KEY_PRESSED(VK_LBUTTON)) {
		Vec2 mouseDelta = InputMgr::Inst()->GetMouseDelta();
		mPlayer->Rotate(0.f, mouseDelta.x);
	}
}



void Script_GroundPlayer::Move(DWORD dwDirection)
{
	if (!dwDirection) {
		return;
	}

	const Vec3 dir = Vector3::Normalized(mObject->GetDirection(dwDirection));
	mObject->Translate(dir * mMovementSpeed * DeltaTime());
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
	rsptr<Animator> animator = mObject->GetObj<GameObject>()->GetAnimator();

	if (weaponIdx == 0) {
		animator->GetController()->SetValue("Weapon", 0);
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