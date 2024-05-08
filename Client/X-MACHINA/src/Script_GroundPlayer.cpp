#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Bullet.h"
#include "Script_GroundObject.h"
#include "Script_AimController.h"
#include "Script_Weapon.h"
#include "Script_Weapon_Pistol.h"
#include "Script_Weapon_Rifle.h"
#include "Script_MainCamera.h"

#include "Component/Rigidbody.h"
#include "Component/Camera.h"

#include "Scene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "InputMgr.h"
#include "Timer.h"
#include "Animator.h"
#include "AnimationClip.h"
#include "AnimatorMotion.h"
#include "AnimatorController.h"


#include "Component/UI.h"

#pragma region Variable
const float Script_GroundPlayer::mkSitWalkSpeed   = 1.5f;
const float Script_GroundPlayer::mkStandWalkSpeed = 2.2f;
const float Script_GroundPlayer::mkRunSpeed       = 5.f;
const float Script_GroundPlayer::mkSprintSpeed    = 8.f;

const float Script_GroundPlayer::mkStartRotAngle = 40.f;

namespace {
	constexpr int kDrawFrame = 13;	// the hand is over the shoulder
}
#pragma endregion






void Script_GroundPlayer::Awake()
{
	base::Awake();

	// add scripts //
	mObject->AddComponent<Script_GroundObject>();
	mSpineBone = mObject->FindFrame("Humanoid_ Spine1");

	mAnimator = mObject->GetObj<GameObject>()->GetAnimator();
	if (mAnimator) {
		mController = mAnimator->GetController();
	}

	mAimController = mObject->AddComponent<Script_AimController>();
}

void Script_GroundPlayer::Start()
{
	base::Start();

	mPlayerType = PlayerType::Human;
	mRotationSpeed = 360.f;

	SetSpawn(Vec3(100, 0, 100));
	SetHP(150.f);
}


void Script_GroundPlayer::Update()
{
	base::Update();

	ProcessInput();

	RecoverRecoil();
}

void Script_GroundPlayer::LateUpdate()
{
	base::LateUpdate();

	RotateMuzzleToAim();
}


void Script_GroundPlayer::UpdateParams(Dir dir, float v, float h, float rotAngle)
{
	if (mIsAim) {										// 에임 상태라면, 플레이어의 look방향에 따라 다른 다리(Legs) 애니메이션을 재생한다.

		const float rotAngleAbs = fabs(rotAngle);
		// 이동 상태 //
		if (!Math::IsZero(v) || !Math::IsZero(h)) {
			const Vec3 movementDir = Transform::GetWorldDirection(dir);

			const float lookAngle = Vector3::SignedAngle(mObject->GetLook().xz(), Vector3::Forward, Vector3::Up);
			Vec3 rotatedMovementDir = Vector3::Normalized(Vector3::Rotate(movementDir, Vector3::Up, lookAngle));

			// v, h값 재계산, dir의 크기가 항상 정사각형 변까지 닿도록 한다.
			// BlendTree의 각 점은 정사각형의 변에 위치해 있기 때문이다.
			// 아래 계산을 하지 않으면 부정확한 v, h값이 구해진다. (조금 부자연스러움)
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
		// 정지 상태에서 회전 //
		else if (rotAngleAbs > 10.f) {
			mController->SetValue("Walk", true);

			// 회전 부호에 따라 h값을 설정한다.
			v = Math::Sign(rotAngle) > 0 ? 0.5f : -0.5f;
			h = Math::Sign(rotAngle) > 0 ? -1.f : 1.f;
		}
		// 정지 상태 //
		else {
			mController->SetValue("Walk", false);
		}
	}
	else {
		if (!Math::IsZero(v) || !Math::IsZero(h)) {		// 에임 상태가 아니면 무조건 앞으로 이동한다.
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

	// 키 입력에 따른 방향 설정 //
	Dir dir{};
	float v{}, h{};
	if (KEY_PRESSED('W')) { v += 1; }
	if (KEY_PRESSED('S')) { v -= 1; }
	if (KEY_PRESSED('A')) { h -= 1; }
	if (KEY_PRESSED('D')) { h += 1; }

	dir |= Math::IsZero(v) ? Dir::None : (v > 0) ? Dir::Front : Dir::Back;
	dir |= Math::IsZero(h) ? Dir::None : (h > 0) ? Dir::Right : Dir::Left;

	UpdateMovement(dir);

	Move(dir);

	float rotAngle = 0.f;
	// rotation //
	if (!mIsAim) {
		RotateTo(dir);
	}
	else {
		RotateToAim(dir, rotAngle);
	}

	MoveCamera(dir);

	// legs blend tree animation //
	if (mController) {
		UpdateParams(dir, v, h, rotAngle);
		mController->SetValueOnly("Vertical", fabs(mParamV) > 0.1f ? mParamV : 0.f);
		mController->SetValueOnly("Horizontal", fabs(mParamH) > 0.1f ? mParamH : 0.f);
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
	if (fabs(angle) > smoothAngleBound) {
		mObject->Rotate(0, Math::Sign(angle) * mRotationSpeed * DeltaTime(), 0);
	}
	else if (fabs(angle) > FLT_EPSILON) {
		mObject->Rotate(0, angle, 0);
	}
}



void Script_GroundPlayer::OnCollisionStay(Object& other)
{
	switch (other.GetTag())
	{
	case ObjectTag::Building:
		//Explode();
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
		OnAim();
		break;

	case WM_RBUTTONUP:
		OffAim();
		break;

	default:
		break;
	}
}

void Script_GroundPlayer::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	base::ProcessKeyboardMsg(messageID, wParam, lParam);

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


void Script_GroundPlayer::StartReload()
{
	if (!mWeapon) {
		return;
	}

	if (mController) {
		mController->SetValue("Reload", true);
	}
}

void Script_GroundPlayer::EndReload()
{
	mWeaponScript->EndReload();

	if (mController) {
		mController->SetValue("Reload", false);
	}
}

void Script_GroundPlayer::BulletFired()
{
	constexpr float recoilAmount = 5.f;
	mCurRecoil += recoilAmount;
	if (fabs(mCurRecoil) >= mMaxRecoil) {
		mCurRecoil = mMaxRecoil;
	}
}



void Script_GroundPlayer::InitWeapons()
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


	const std::unordered_map<WeaponType, std::string> reloadMotions{
		{WeaponType::HandedGun, "Reload2HandedGun" },
		{WeaponType::AssaultRifle, "ReloadAssaultRifle" },
		{WeaponType::LightingGun, "ReloadOverheatBeamGun" },
		{WeaponType::GatlinGun, "ReloadOverheatGatlinGun" },
		{WeaponType::ShotGun, "ReloadShotgun" },
		{WeaponType::MissileLauncher, "ReloadMissileLauncher" },
	};

	const std::unordered_map<WeaponType, std::string> drawMotions{
		{WeaponType::HandedGun, "Draw2HandedGun" },
		{WeaponType::AssaultRifle, "DrawAssaultRifle" },
		{WeaponType::LightingGun, "DrawBeamGun" },
		{WeaponType::GatlinGun, "DrawGatlinGun" },
		{WeaponType::ShotGun, "DrawShotgun" },
		{WeaponType::MissileLauncher, "DrawMissileLauncher" },
	};

	const std::unordered_map<WeaponType, std::string> putbackMotions{
		{WeaponType::HandedGun, "PutBack2HandedGun" },
		{WeaponType::AssaultRifle, "PutBackAssaultRifle" },
		{WeaponType::LightingGun, "PutBackBeamGun" },
		{WeaponType::GatlinGun, "PutBackGatlinGun" },
		{WeaponType::ShotGun, "PutBackShotgun" },
		{WeaponType::MissileLauncher, "PutBackMissileLauncher" },
	};

	const std::unordered_map<WeaponType, std::string> shootMotions{
		{WeaponType::HandedGun, "ShootPrimary2HandedGun" },
		{WeaponType::AssaultRifle, "ShootPrimaryAssaultRifle" },
		{WeaponType::LightingGun, "ShootPrimaryBeamGun" },
		{WeaponType::GatlinGun, "ShootPrimaryGatlinGun" },
		{WeaponType::ShotGun, "ShootPrimaryShotgun" },
		{WeaponType::MissileLauncher, "ShootPrimaryMissileLauncher" },
	};

	std::function<void()> reloadCallback       = std::bind(&Script_GroundPlayer::EndReloadCallback, this);
	std::function<void()> reloadStopCallback   = std::bind(&Script_GroundPlayer::StopReloadCallback, this);
	std::function<void()> reloadChangeCallback = std::bind(&Script_GroundPlayer::ChangeReloadCallback, this);
	std::function<void()> drawCallback         = std::bind(&Script_GroundPlayer::DrawWeaponCallback, this);
	std::function<void()> drawEndCallback      = std::bind(&Script_GroundPlayer::DrawWeaponEndCallback, this);
	std::function<void()> putbackCallback      = std::bind(&Script_GroundPlayer::PutbackWeaponEndCallback, this);

	mWeapons.resize(gkWeaponTypeCnt, nullptr);
	for (size_t i = 0; i < gkWeaponTypeCnt; ++i) {
		// weapon 타입에 따른 객체 생성 //
		auto& weapon = mWeapons[i];
		WeaponType weaponType = static_cast<WeaponType>(i);
		weapon = Scene::I->Instantiate(defaultWeapons.at(weaponType), false);
		if (!weapon) {
			continue;
		}

		// weaponType에 따른 스크립트 추가 //
		switch (weaponType) {
		case WeaponType::HandedGun:
		case WeaponType::LightingGun:
		case WeaponType::GatlinGun:
		case WeaponType::ShotGun:
		case WeaponType::MissileLauncher:
			weapon->AddComponent<Script_Weapon_Pistol>();
			break;
		case WeaponType::AssaultRifle:
			weapon->AddComponent<Script_Weapon_Skyline>();
			break;
		default:
			assert(0);
			break;
		}

		// 스크립트 설정 //
		weapon->GetComponent<Script_Weapon>()->SetOwner(this);

		// transform 설정 //
		Transform* transform = mObject->FindFrame(defaultTransforms.at(weaponType));
		if (!transform) {
			continue;
		}
		transform->SetChild(weapon);

		// setting callbacks //
		constexpr int kPutbackFrame = 15;	// the hand is over the shoulder

		auto& realodMotion  = mReloadMotions[static_cast<int>(weaponType)] = mController->FindMotionByName(reloadMotions.at(weaponType), "Body");
		auto& drawMotion    = mController->FindMotionByName(drawMotions.at(weaponType), "Body");
		auto& putbackMotion = mController->FindMotionByName(putbackMotions.at(weaponType), "Body");
		auto& shootMotion   = mController->FindMotionByName(shootMotions.at(weaponType), "Body");

		// sync off
		realodMotion->SetSync(false);
		drawMotion->SetSync(false);
		putbackMotion->SetSync(false);
		shootMotion->SetSync(false);

		// add callbacks
		realodMotion->AddCallback(reloadCallback, realodMotion->GetMaxFrameRate() - 10); // for smooth animation, reload complete before [10] frame
		realodMotion->AddStopCallback(reloadStopCallback);
		realodMotion->AddChangeCallback(reloadChangeCallback);
		drawMotion->AddCallback(drawCallback, kDrawFrame);
		drawMotion->AddEndCallback(drawEndCallback);
		putbackMotion->AddCallback(putbackCallback, kPutbackFrame);


		weapon->Awake();
	}
}

void Script_GroundPlayer::DrawWeaponStart(int weaponIdx, bool isDrawImmed)
{
	base::DrawWeaponStart(weaponIdx, isDrawImmed);

	mController->SetValue("Weapon", weaponIdx);

	// synchronize animation frame
	// pistol's animation is different. so can't synchronize with others
	constexpr int kPistolIndex = 1;
	if (isDrawImmed && weaponIdx != kPistolIndex && GetCrntWeaponIdx() != kPistolIndex) {
		mController->SetValue("Draw", true, true);
		auto& motion = mController->GetCrntMotion("Body");
		motion->SetLength(motion->GetClip()->GetFrameTime(kDrawFrame));
	}
	else {
		mController->SetValue("Draw", true, false);
	}

	ResetAimingTime();
}

void Script_GroundPlayer::DrawWeaponCallback()
{
	base::DrawWeapon();

	auto& motion            = mReloadMotions[static_cast<int>(mWeaponScript->GetWeaponType())];
	const float reloadTime  = mWeaponScript->GetReloadTime();
	const float motionTime  = motion->GetMaxLength();
	const float realodSpeed = motionTime / reloadTime;
	motion->ResetOriginSpeed(realodSpeed);
}

void Script_GroundPlayer::DrawWeaponEndCallback()
{
	base::DrawWeaponEnd();

	if (mIsAim) {
		mController->SetValue("Draw", false, false);
		mController->SetValue("Aim", false, true);
	}
	else {
		mController->SetValue("Draw", false, true);
	}

	if (KEY_PRESSED(VK_RBUTTON)) {
		OnAim();
	}
}

void Script_GroundPlayer::PutbackWeapon()
{
	base::PutbackWeapon();

	if (mWeapon) {
		StopReload();
		mController->SetValue("PutBack", true);
	}
}

void Script_GroundPlayer::PutbackWeaponEndCallback()
{
	mController->SetValue("Weapon", 0);

	base::PutbackWeaponEnd();

	// 다음 무기가 없다면 조준 상태를 종료한다.
	if (GetNextWeaponIdx() == -1 && mIsAim) {
		OffAim();
	}
	mController->SetValue("PutBack", false);
}


void Script_GroundPlayer::UpdateParam(float val, float& param)
{
	constexpr float kParamSpeed         = 6.f;		// 파라미터 전환 속도
	constexpr float kOppositeExtraSpeed = 8.f;		// 반대편 이동 시 추가 이동 전환 속도

	int sign = Math::Sign(val);						// sign : 파라미터 이동 방향 = 현재 입력값의 부호
	if (Math::IsZero(val)) {						//		  입력이 없다면 현재 파라미터의 반대 부호
		if (Math::IsZero(param)) {
			return;
		}
		sign = -Math::Sign(param);
	}
	float before = param;
	param += (kParamSpeed * sign) * DeltaTime();	// 파라미터값 증감

	if (!Math::IsZero(val)) {
		if (fabs(param) < 0.5f && (fabs(before) > fabs(param))) {	// 반대편 이동 시
			param += (sign * kOppositeExtraSpeed) * DeltaTime();	// 추가 전환 속도 적용
		}
		else if (fabs(param) >= fabs(before)) {						// 정방향 이동 시
			param = std::clamp(param, -fabs(val), fabs(val));		// param이 val을 넘지 못하도록 한다.

			// 증감폭이 낮고 && 0에 가까운 경우 해당 파라미터는 무시한다.
			if (fabs(fabs(param) - fabs(before)) < 0.001f && fabs(param) < 0.1f) {								// 0에 근접하면 0으로 설정
				param = 0.f;
			}
		}
	}

	param = std::clamp(param, -1.f, 1.f);		// -1 ~ 1 사이로 고정
}



void Script_GroundPlayer::UpdateMovement(Dir dir)
{
	// 현재 캐릭터의 움직임 상태를 키 입력에 따라 설정한다.
	PlayerMotion crntMovement = PlayerMotion::None;
	// Stand / Sit
	if (KEY_PRESSED(VK_CONTROL)) { crntMovement |= PlayerMotion::Sit; }
	else { crntMovement |= PlayerMotion::Stand; }
	// Walk / Run / Sprint
	if (dir != Dir::None) {
		if (mIsAim) {
			crntMovement |= PlayerMotion::Walk;
		}
		else {
			if (KEY_PRESSED(VK_SHIFT)) { crntMovement |= PlayerMotion::Sprint; }
			else if (KEY_PRESSED('C')) { crntMovement |= PlayerMotion::Walk; }
			else { crntMovement |= PlayerMotion::Run; }
		}
	}

	PlayerMotion prevState = PlayerMotion::GetState(mPrevMovement);
	PlayerMotion prevMotion = PlayerMotion::GetMotion(mPrevMovement);

	PlayerMotion crntState = PlayerMotion::GetState(crntMovement);
	PlayerMotion crntMotion = PlayerMotion::GetMotion(crntMovement);

	SetState(prevState, prevMotion, crntState);
	SetMotion(prevState, prevMotion, crntState, crntMotion);

	mPrevMovement = crntState | crntMotion;
}




float Script_GroundPlayer::GetAngleMuzzleToAim(const Vec3& aimWorldPos) const
{
	// 회전축
	const Vec3 offsetPos = mSpineBone->GetPosition().xz();

	// 총구 위치&방향
	const Vec3 muzzlePos = mMuzzle->GetPosition().xz();
	const Vec3 muzzleLook = mMuzzle->GetLook().xz();

	// 회전축에서 조준점으로 향하는 벡터
	const Vec3 offsetToAim = aimWorldPos - offsetPos;

	// 총구에서 탄착점으로 향하는 벡터의 길이 근사값
	const float approxLength = (offsetToAim.Length() - (muzzlePos - offsetPos).Length());

	// 탄착점으로 향하는 벡터
	const Vec3 muzzleToBullet = muzzlePos + (muzzleLook * approxLength);

	// 회전축에서 탄착점으로 향하는 벡터
	const Vec3 offsetToBullet = muzzleToBullet - offsetPos;

	// (회전축-탄착점) -> (회전축-조준점)으로 향하는 각도
	return Vector3::SignedAngle(offsetToBullet, offsetToAim, Vector3::Up);
}

float Script_GroundPlayer::GetAngleSpineToAim(const Vec3& aimWorldPos) const
{
	return Vector3::SignedAngle(mSpineBone->GetUp().xz(), aimWorldPos.xz() - mSpineBone->GetPosition().xz(), Vector3::Up);;
}

Vec3 Script_GroundPlayer::GetAimWorldPos(const Vec2& aimScreenPos) const
{
	// aim에서 발사된 광선에서 총구의 y값과 일치하는 지점을 찾는다.
	const Vec3 ray = MAIN_CAMERA->ScreenToWorldRay(aimScreenPos);
	const Vec3 camPos = MAIN_CAMERA->GetPosition();
	return Vector3::RayOnPoint(camPos, ray, mMuzzle->GetPosition().y).xz();
}

void Script_GroundPlayer::RotateToAim(Dir dir, float& rotAngle)
{
	constexpr float kStopRotAngle = 10.f;
	const Vec2 aimDir = mAimController->GetAimDirection();

	bool moving = dir != Dir::None;
	// spine bone's look vector is aim direction (spine bone gonna look at aim from LateUpdate function)
	// get an angle if end the aim animation
	if (!moving && !IsInGunChangeMotion()) {
		rotAngle = mCrntSpineAngle;
	}
	else {
		rotAngle = Vector3::SignedAngle(mObject->GetLook().xz(), Vec3(aimDir.x, 0, aimDir.y), Vector3::Up);
	}

	// look at the aim if moving
	if (moving) {
		Rotate(rotAngle);
	}
	// rotate body to the aim if spine bone's angle too large
	else if (mIsInBodyRotation) {
		if (fabs(rotAngle) < kStopRotAngle) {
			mIsInBodyRotation = false;
			rotAngle = 0.f;
			return;
		}

		Rotate(mSpineDstAngle);
	}
	else {
		rotAngle = 0.f;
	}
}


void Script_GroundPlayer::Rotate(float angle) const
{
	constexpr float kMaxAngle = 90.f;

	const int sign = Math::Sign(angle);
	const float angleAbs = fabs(angle);

	// [kMaxAngle] 각도 이하면 보간
	float rotationSpeed{};
	if (angleAbs > kMaxAngle) {
		rotationSpeed = mRotationSpeed;
	}
	else {
		rotationSpeed = (angleAbs / kMaxAngle) * mRotationSpeed;	// interpolation for smooth rotation
	}

	mObject->Rotate(0, sign * rotationSpeed * DeltaTime(), 0);
}


void Script_GroundPlayer::RotateMuzzleToAim()
{
	// keep the muzzle facing the target //
	constexpr float kAimingSpeed  = 10.f;
	constexpr float kHoldingSpeed = 5.f;

	if (mIsAim && mMuzzle) {
		if (IsInGunChangeMotion()) {
			return;
		}

		// angle could be too large if aim is so close
		constexpr float kAimMinDistance = 300.f;
		Vec2 aimScreenPos = mAimController->GetAimPos();
		if (aimScreenPos.Length() < kAimMinDistance) {
			aimScreenPos = Vector2::Normalized(aimScreenPos) * kAimMinDistance;
		}

		// smoothly rotate the spin angle through linear interpolation.
		::IncreaseDelta(mAimingDeltaTime, kAimingSpeed);

		const Vec3 aimWorldPos = GetAimWorldPos(aimScreenPos);
		// 재장전 중이라면, 총구가 아닌 척추의 방향에 따라 회전각을 구한다.
		if (mWeaponScript->IsReloading()) {
			// 현재 각도를 유지하며 [mReloadingDeltaTime]이 1이 될 때 까지 서서히 회전한다.
			if (::IncreaseDelta(mReloadingDeltaTime, kAimingSpeed)) {

				mSpineBone->RotateGlobal(Vector3::Up, mCrntSpineAngle);
				float angle = GetAngleSpineToAim(aimWorldPos) * mAimingDeltaTime;
				mSpineBone->RotateGlobal(Vector3::Up, -mCrntSpineAngle);
				mCrntSpineAngle += angle * mReloadingDeltaTime;
			}
			else {
				mCrntSpineAngle = GetAngleSpineToAim(aimWorldPos) * mAimingDeltaTime;
			}
		}
		// 재장전 중이었다면 현재 각도를 유지하며 목표 지점까지 서서히 회전한다.
		else {
			if (::DecreaseDelta(mReloadingDeltaTime, kAimingSpeed)) {

				mSpineBone->RotateGlobal(Vector3::Up, mCrntSpineAngle);
				float angle = GetAngleMuzzleToAim(aimWorldPos) * mAimingDeltaTime;
				mSpineBone->RotateGlobal(Vector3::Up, -mCrntSpineAngle);
				mCrntSpineAngle += angle * (1 - mReloadingDeltaTime);
			}
			else {
				mCrntSpineAngle = GetAngleMuzzleToAim(aimWorldPos) * mAimingDeltaTime;
			}
		}

		if (fabs(mCrntSpineAngle) > 0.1f) {
			mSpineBone->RotateGlobal(Vector3::Up, mCrntSpineAngle);

			if(mCurRecoil > 0.f) {
				mSpineBone->Rotate(Vector3::Forward, mCurRecoil);
			}

			// spine angle is max [mkStartRotAngle] degree from object direction, so can't rotate anymore //
			constexpr float leftAngleBound = 15.f;	// can rotate more to the left
			const float correctedSpineAngle = mCrntSpineAngle + leftAngleBound;
			if (fabs(correctedSpineAngle) > mkStartRotAngle) {
				const int sign = Math::Sign(mCrntSpineAngle);
				const float corrAngle = (fabs(correctedSpineAngle) - mkStartRotAngle) * -sign;
				mSpineDstAngle = correctedSpineAngle;
				mCrntSpineAngle += corrAngle;
				mSpineBone->RotateGlobal(Vector3::Up, corrAngle);

				mIsInBodyRotation = true;
			}
		}
	}
	else if (::DecreaseDelta(mAimingDeltaTime, kHoldingSpeed)) {
		mSpineBone->RotateGlobal(Vector3::Up, mCrntSpineAngle * mAimingDeltaTime);
		mCrntSpineAngle *= mAimingDeltaTime;
	}
}


void Script_GroundPlayer::OnAim()
{
	if (!mWeapon || !mController) {
		return;
	}
	mController->SetValue("Aim", true);
	mIsAim = true;
}

void Script_GroundPlayer::OffAim()
{
	mController->SetValue("Aim", false);
	mIsAim = false;

	// 에임 도중 회전상태였다면 이를 취소한다.
	if (mIsInBodyRotation) {
		mIsInBodyRotation = false;

		PlayerMotion prevMotion = mPrevMovement & 0xF0;
		if (prevMotion == PlayerMotion::None) {
			mController->SetValue("Walk", false);
		}
	}
}

void Script_GroundPlayer::Reload()
{
	base::Reload();

	StartReload();
}

void Script_GroundPlayer::StopReload()
{
	if (mController) {
		mController->SetValue("Reload", false);
	}
}

void Script_GroundPlayer::SetState(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState)
{
	// 이전 움직임 상태와 다른 경우만 값을 업데이트 한다.
	// 이전 상태를 취소하고 현재 상태로 전환한다.
	if (!(crntState & prevState)) {
		switch (prevState) {
		case PlayerMotion::None:
		case PlayerMotion::Stand:
			break;
		case PlayerMotion::Sit:
			mController->SetValue("Sit", false);
			break;

		default:
			assert(0);
			break;
		}

		switch (crntState) {
		case PlayerMotion::None:
			break;
		case PlayerMotion::Stand:
		{
			switch (prevMotion) {
			case PlayerMotion::None:
				break;
			case PlayerMotion::Walk:
				mMovementSpeed = mkStandWalkSpeed;
				mController->SetValue("Walk", true);
				break;
			case PlayerMotion::Run:
				mMovementSpeed = mkRunSpeed;
				mController->SetValue("Run", true);
				break;
			case PlayerMotion::Sprint:
				mMovementSpeed = mkSprintSpeed;
				mController->SetValue("Sprint", true);
				break;
			default:
				assert(0);
				break;
			}
		}
			break;
		case PlayerMotion::Sit:
			mController->SetValue("Sit", true);
			mMovementSpeed = mkSitWalkSpeed;
			break;

		default:
			assert(0);
			break;
		}
	}
}

void Script_GroundPlayer::SetMotion(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState, PlayerMotion& crntMotion)
{
	// 이전 상태의 모션을 취소하고 현재 상태의 모션으로 전환한다.
	if (!(crntState & prevState) || !(crntMotion & prevMotion)) {
		switch (prevMotion) {
		case PlayerMotion::None:
			break;
		case PlayerMotion::Walk:
			mController->SetValue("Walk", false);
			break;
		case PlayerMotion::Run:
			mController->SetValue("Run", false);
			break;
		case PlayerMotion::Sprint:
			mController->SetValue("Sprint", false);
			break;

		default:
			assert(0);
			break;
		}

		switch (crntMotion) {
		case PlayerMotion::None:
			break;
		case PlayerMotion::Walk:
			break;

		case PlayerMotion::Run:
			if (crntState & PlayerMotion::Sit) {
				crntMotion = PlayerMotion::Walk;
			}

			break;
		case PlayerMotion::Sprint:
			if (crntState & PlayerMotion::Sit) {
				crntMotion = PlayerMotion::Walk;
			}

			break;
		default:
			assert(0);
			break;
		}
	}

	switch (crntMotion) {
	case PlayerMotion::None:
		mMovementSpeed = 0.f;
		break;
	case PlayerMotion::Walk:
		mController->SetValue("Walk", true);
		if (crntState & PlayerMotion::Stand) {
			mMovementSpeed = mkStandWalkSpeed;
		}
		else {
			mMovementSpeed = mkSitWalkSpeed;
		}
		break;
	case PlayerMotion::Run:
		mController->SetValue("Run", true);
		mMovementSpeed = mkRunSpeed;
		break;
	case PlayerMotion::Sprint:
		mController->SetValue("Sprint", true);
		mMovementSpeed = mkSprintSpeed;
		break;

	default:
		assert(0);
		break;
	}
}

void Script_GroundPlayer::StopReloadCallback()
{
	StopReload();
}

void Script_GroundPlayer::ChangeReloadCallback()
{
	const auto& motion = mController->GetCrntMotion("Body");
	float ratio = motion->GetLength() / motion->GetMaxLength();

	// 리로드 도중 모션 변경 시 80%이상 진행되었다면 장전 완료 처리
	// 아직 재장전 상태인 경우만 적용
	constexpr float kAllowRatio = 0.8f;
	if (ratio > kAllowRatio, mController->GetParamValue<bool>("Reload") == true) {
		EndReload();
	}
}

void Script_GroundPlayer::EndReloadCallback()
{
	EndReload();
}

void Script_GroundPlayer::RecoverRecoil()
{
	constexpr float recoverAmount = 70.f;

	if (mCurRecoil > 0.f) {
		mCurRecoil -= recoverAmount * DeltaTime();
		if (mCurRecoil <= 0.f) {
			mCurRecoil = 0.f;
		}
	}
}

void Script_GroundPlayer::MoveCamera(Dir dir)
{
	if (mIsAim) {
		const Vec2 mousePos  = mAimController->GetAimPos();
		const Vec2 ndc       = MAIN_CAMERA->ScreenToNDC(mousePos);
		const float offset_t = (std::max)(fabs(ndc.x), fabs(ndc.y));

		mCamera->Move(mousePos, offset_t);
	}
	else if(dir != Dir::None) {
		const Vec3 dirVec = Transform::GetWorldDirection(dir);

		mCamera->Move(Vec2(dirVec.x, dirVec.z), 0.6f, true);
	}
}