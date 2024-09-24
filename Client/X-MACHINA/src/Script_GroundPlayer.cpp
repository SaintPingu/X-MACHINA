#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Bullet.h"
#include "Script_GroundObject.h"
#include "Script_AimController.h"
#include "Script_MainCamera.h"
#include "Script_Weapon.h"
#include "Script_Weapon_Pistol.h"
#include "Script_Weapon_Rifle.h"
#include "Script_Weapon_Shotgun.h"
#include "Script_Weapon_Sniper.h"
#include "Script_Weapon_MissileLauncher.h"
#include "Script_Weapon_MineLauncher.h"
#include "Script_Item.h"
#include "Script_BattleUI.h"

#include "Component/Rigidbody.h"
#include "Component/Camera.h"
#include "Component/Collider.h"

#include "BattleScene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "InputMgr.h"
#include "SoundMgr.h"
#include "Timer.h"
#include "Animator.h"
#include "AnimationClip.h"
#include "AnimatorMotion.h"
#include "AnimatorController.h"

#include "Component/UI.h"

#include "ClientNetwork/Contents/ClientNetworkManager.h"
#include "ClientNetwork/Contents/FBsPacketFactory.h"


#pragma region Variable
const float Script_GroundPlayer::mkSitWalkSpeed   = 1.5f;
const float Script_GroundPlayer::mkStandWalkSpeed = 2.2f;
const float Script_GroundPlayer::mkRunSpeed       = 5.f;
const float Script_GroundPlayer::mkSprintSpeed    = 8.f;

const float Script_GroundPlayer::mkStartRotAngle = 40.f;

namespace {
	constexpr int kPistolNum = 1;
	constexpr int kDrawFrame = 13;	// the hand is over the shoulder

	static const std::unordered_map<WeaponType, std::string> kDefaultTransforms{
		{WeaponType::HandedGun, "RefPos2HandedGun_Action" },
		{WeaponType::AssaultRifle, "RefPosAssaultRifle_Action" },
		{WeaponType::ShotGun, "RefPosShotgun_Action" },
		{WeaponType::MissileLauncher, "RefPosMissileLauncher_Action" },
		{WeaponType::Sniper, "RefPosSniper_Action" },
		{WeaponType::MineLauncher, "RefPosLightningGun_Action" },
	};
}
#pragma endregion






bool Script_GroundPlayer::IsReloading() const
{
	return mController->GetParamValue<bool>("Reload");
}

void Script_GroundPlayer::Awake()
{
	base::Awake();

	// add scripts //
	mObject->AddComponent<Script_GroundObject>();

	// values //
	mSpineBone = mObject->FindFrame("Humanoid_ Spine1");
	SetMaxHP(1000.f);

	// animations //
	mAnimator = mObject->GetObj<GameObject>()->GetAnimator();
	assert(mAnimator);
	mController = mAnimator->GetController();
	mController->SetPlayer();
	InitWeaponAnimations();

	// weapons //
	mWeapons.resize(3);
	AquireNewWeapon(WeaponName::H_Lock);
}

void Script_GroundPlayer::Start()
{
	base::Start();

	mRotationSpeed = 360.f;

	//constexpr Vec3 kSpawnPoint = Vec3(450, 0, 240); // mining mech
	//constexpr Vec3 kSpawnPoint = Vec3(100, 0, 210); // start
	//constexpr Vec3 kSpawnPoint = Vec3(250, 0, 210); // ursacetus
	constexpr Vec3 kSpawnPoint = Vec3(47, 0, 230); // base camp
	//constexpr Vec3 kSpawnPoint = Vec3(630, 0, 315); // deus
	//constexpr Vec3 kSpawnPoint = Vec3(530, 0, 315); // stage3

	SetSpawn(kSpawnPoint);
	mObject->SetPosition(kSpawnPoint);
}


void Script_GroundPlayer::Update()
{
	base::Update();

	RecoverRecoil();
}

void Script_GroundPlayer::LateUpdate()
{
	base::LateUpdate();

	RotateMuzzleToAim();
}

void Script_GroundPlayer::OnDestroy()
{
	base::OnDestroy();

	if (mController) {
		mController->Release();
	}
}

void Script_GroundPlayer::OnCollisionStay(Object& other)
{
	base::OnCollisionStay(other);

	switch (other.GetTag())
	{
	case ObjectTag::Building:
	case ObjectTag::DissolveBuilding:
	case ObjectTag::Bound:
		ComputeSlideVector(other);
		break;
	default:
		break;
	}
}


void Script_GroundPlayer::UpdateParams(Dir dir, float v, float h, float rotAngle)
{
	if (mIsAim) { // In the aim state, play a different leg animation depending on the player's look direction.

		const float rotAngleAbs = fabs(rotAngle);
		// Rotating in a moving state //
		if (!Math::IsZero(v) || !Math::IsZero(h)) {
			const Vec3 movementDir = Transform::GetWorldDirection(dir);

			const float lookAngle = Vector3::SignedAngle(mObject->GetLook().xz(), Vector3::Forward, Vector3::Up);
			Vec3 rotatedMovementDir = Vector3::Normalized(Vector3::Rotate(movementDir, Vector3::Up, lookAngle));

			// Recalculating the values of v and h //
			// Always make sure that the dir size reaches the square side.
			// This is because each point of BlendTree is located on the side of a square.
			// If you don't do the following calculations, you'll get the incorrect values of v and h. (a little unnatural)
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
		// Rotating in a stationary state //
		else if (rotAngleAbs > 10.f) {
			mController->SetValue("Walk", true);

			// Set the h value according to the rotation sign.
			v = Math::Sign(rotAngle) > 0 ? 0.5f : -0.5f;
			h = Math::Sign(rotAngle) > 0 ? -1.f : 1.f;
		}
		// Stationary state //
		else {
			mController->SetValue("Walk", false);
		}
	}
	else {
		if (!Math::IsZero(v) || !Math::IsZero(h)) { // If it's not in an aim state, it moves forward unconditionally.
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

	if (KEY_PRESSED('O')) mCamera->ZoomOut();
	if (KEY_PRESSED('P')) mCamera->ZoomIn();
	if (KEY_PRESSED('I')) mCamera->ZoomReset();
}

bool Script_GroundPlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!base::ProcessMouseMsg(messageID, wParam, lParam)) {
		return false;
	}

	switch (messageID) {
	case WM_RBUTTONDOWN:
		OnAim();
		break;

	case WM_RBUTTONUP:
		OffAim();
		break;
	}

	return true;
}

void Script_GroundPlayer::Move(Dir dir)
{
	if (mCurrPos.Length() == 0)
	{
		mCurrPos = mObject->GetPosition();
		mPrevPos = mObject->GetPosition();

	}

	mPrevPos = mObject->GetPosition();

	if (dir == Dir::None) {
		return;
	}

	const Vec3 dirVec = Transform::GetWorldDirection(dir);
	mDirVec = dirVec;

	if (mSlideVec != Vector3::One) {
		mObject->Translate(mSlideVec * mMovementSpeed / 1.5f * DeltaTime());
		mSlideVec = Vector3::One;
	}
	else {
		mObject->Translate(dirVec * mMovementSpeed * DeltaTime());
	}

	mCurrPos = mObject->GetPosition();

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


void Script_GroundPlayer::InitWeaponAnimations()
{
	static const std::unordered_map<WeaponType, std::string> kReloadMotions{
		{WeaponType::HandedGun, "Reload2HandedGun" },
		{WeaponType::AssaultRifle, "ReloadAssaultRifle" },
		{WeaponType::ShotGun, "ReloadShotgun" },
		{WeaponType::MissileLauncher, "ReloadMissileLauncher"  },
		{WeaponType::Sniper, "ReloadAssaultRifle" },
		{WeaponType::MineLauncher, "ReloadOverheatBeamGun" },
	};

	static const std::unordered_map<WeaponType, std::string> kDrawMotions{
		{WeaponType::HandedGun, "Draw2HandedGun" },
		{WeaponType::AssaultRifle, "DrawAssaultRifle" },
		{WeaponType::ShotGun, "DrawShotgun" },
		{WeaponType::MissileLauncher, "DrawMissileLauncher" },
		{WeaponType::Sniper, "DrawAssaultRifle" },
		{WeaponType::MineLauncher, "DrawBeamGun" },
	};

	static const std::unordered_map<WeaponType, std::string> kPutbackMotions{
		{WeaponType::HandedGun, "PutBack2HandedGun" },
		{WeaponType::AssaultRifle, "PutBackAssaultRifle" },
		{WeaponType::ShotGun, "PutBackShotgun" },
		{WeaponType::MissileLauncher, "PutBackMissileLauncher" },
		{WeaponType::Sniper, "PutBackAssaultRifle" },
		{WeaponType::MineLauncher, "PutBackBeamGun" },
	};

	// animation callback functions //
	const std::function<void()>& reloadCallback = std::bind(&Script_GroundPlayer::EndReloadCallback, this);
	const std::function<void()>& reloadStopCallback = std::bind(&Script_GroundPlayer::StopReloadCallback, this);
	const std::function<void()>& reloadChangeCallback = std::bind(&Script_GroundPlayer::ChangeReloadCallback, this);
	const std::function<void()>& drawCallback = std::bind(&Script_GroundPlayer::DrawWeaponCallback, this);
	const std::function<void()>& drawEndCallback = std::bind(&Script_GroundPlayer::DrawWeaponEndCallback, this);
	const std::function<void()>& putbackCallback = std::bind(&Script_GroundPlayer::PutbackWeaponEndCallback, this);

	// init bolt action sniper
	{
		const std::function<void()>& boltActionCallback = std::bind(&Script_GroundPlayer::BoltActionCallback, this);
		const std::function<void()>& boltActionSoundCallback = std::bind(&Script_GroundPlayer::BoltActionSoundCallback, this);
		const auto& boltActionMotion = mController->FindMotionByName("BoltActionSniper", "Body");

		// callback
		boltActionMotion->AddEndCallback(boltActionCallback);
		boltActionMotion->AddChangeCallback(boltActionCallback);
		boltActionMotion->AddCallback(boltActionSoundCallback, 20);
	}

	
	// setting callbacks //
	constexpr int kPutbackFrame = 15;	// the hand is over the shoulder

	for (int i = 0; i < static_cast<int>(WeaponName::_count); ++i) {
		WeaponName weaponName = static_cast<WeaponName>(i);
		WeaponType weaponType = gkWeaponTypeMap.at(weaponName);

		if (!kReloadMotions.count(weaponType)) {
			continue;
		}

		const auto& realodMotion = mReloadMotions[static_cast<int>(weaponName)] = mController->FindMotionByName(kReloadMotions.at(weaponType), "Body");
		const auto& drawMotion = mController->FindMotionByName(kDrawMotions.at(weaponType), "Body");
		const auto& putbackMotion = mController->FindMotionByName(kPutbackMotions.at(weaponType), "Body");

		// add callbacks
		realodMotion->AddCallback(reloadCallback, realodMotion->GetMaxFrameRate() - 10); // for smooth animation, reload complete before [10] frame
		realodMotion->AddStopCallback(reloadStopCallback);
		realodMotion->AddChangeCallback(reloadChangeCallback);
		drawMotion->AddCallback(drawCallback, kDrawFrame);
		drawMotion->AddEndCallback(drawEndCallback);
		putbackMotion->AddCallback(putbackCallback, kPutbackFrame);
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

	UpdateWeaponUI();
}

void Script_GroundPlayer::BoltAction()
{
	if (!mWeapon) {
		return;
	}

	if (mController) {
		mController->SetValue("BoltAction", true);

		mWeapon->DetachParent(false);
		Transform* leftHand = mObject->FindFrame("Humanoid_ L Hand");
		leftHand->SetChild(mWeapon->GetShared(), false);
	}
}

void Script_GroundPlayer::EndReload()
{
	if (mWeaponScript) {
		mWeaponScript->EndReload();
	}

	if (mController) {
		mController->SetValue("Reload", false);
	}

	UpdateWeaponUI();
}

void Script_GroundPlayer::BulletFired()
{
	base::BulletFired();

	constexpr float recoilAmount = 5.f;
	mCurRecoil += recoilAmount;
	if (fabs(mCurRecoil) >= mMaxRecoil) {
		mCurRecoil = mMaxRecoil;
	}

#ifdef SERVER_COMMUNICATION
	/// +-------------------------------------------------------------------
	///		Send OnShoot Packet
	/// -------------------------------------------------------------------+
	float spineYAngle = Vector3::SignedAngle(Vector3::Forward, mSpineBone->GetUp().xz(), Vector3::Up);
	float objYAngle   = mObject->GetYAngle();

	auto pkt = FBS_FACTORY->CPkt_Player_AimRotation(objYAngle, spineYAngle);
	CLIENT_NETWORK->Send(pkt);
#endif
}



void Script_GroundPlayer::AquireNewWeapon(WeaponName weaponName)
{
	GridObject* weapon = BattleScene::I->Instantiate(Script_Weapon::GetWeaponModelName(weaponName), ObjectTag::Dynamic, false);

	// add scripts //
	switch (weaponName) {
	case WeaponName::H_Lock:
		weapon->AddComponent<Script_Weapon_Pistol>();
		break;
	case WeaponName::DBMS:
		weapon->AddComponent<Script_Weapon_DBMS>();
		break;
	case WeaponName::SkyLine:
		weapon->AddComponent<Script_Weapon_Skyline>();
		break;
	case WeaponName::Burnout:
		weapon->AddComponent<Script_Weapon_Burnout>();
		break;
	case WeaponName::PipeLine:
		weapon->AddComponent<Script_Weapon_PipeLine>();
		ResetBoltActionMotionSpeed(weapon->GetComponent<Script_Weapon>());
		break;
	case WeaponName::MineLauncher:
		weapon->AddComponent<Script_Weapon_MineLauncher>();
		break;
	default:
		assert(0);
		break;
	}
	const auto& weaponScript = weapon->GetComponent<Script_Weapon>();
	weaponScript->Awake();
	weaponScript->SetOwner(this);
	weaponScript->SetPlayerWeapon(true, mObject);

	SwitchWeapon(weapon);
	SetWeaponChild(weapon);
}

void Script_GroundPlayer::TakeWeapon(rsptr<Script_Weapon> weapon)
{
	GridObject* gameObject = weapon->GetObj()->GetObj<GridObject>();
	SwitchWeapon(gameObject);
	SetWeaponChild(gameObject);
	weapon->SetOwner(this);
	weapon->SetPlayerWeapon(true, mObject);
	gameObject->GetComponent<Script_Weapon>()->SetOwner(this);

	if (weapon->GetWeaponType() == WeaponType::Sniper) {
		ResetBoltActionMotionSpeed(weapon);
	}
}


void Script_GroundPlayer::OnAim()
{
	if (!mWeapon || !mController) {
		return;
	}

	SoundMgr::I->Play("Gun", "OnAim");
	mController->SetValue("Aim", true);
	mIsAim = true;
}

void Script_GroundPlayer::OffAim()
{
	mController->SetValue("Aim", false);
	mIsAim = false;

	if (mIsInBodyRotation) {
		mIsInBodyRotation = false;

		PlayerMotion prevMotion = mPrevMovement & 0xF0;
		if (prevMotion == PlayerMotion::None) {
			mController->SetValue("Walk", false);
		}
	}
}

void Script_GroundPlayer::DropCrntWeapon()
{
	DropWeapon(GetCrntWeaponNum() - 1);
}

void Script_GroundPlayer::DropWeapon(int weaponIdx)
{
	if (weaponIdx < 0) {
		return;
	}


	base::DropWeapon(weaponIdx);

	OffAim();

	if (mWeapon == nullptr) {
		ResetWeaponAnimation();
	}
}

void Script_GroundPlayer::DrawWeaponStart(int weaponNum, bool isDrawImmed)
{
	base::DrawWeaponStart(weaponNum, isDrawImmed);

	if (weaponNum <= 0) {
		return;
	}

	int weaponIdx{};
	switch (mWeapons[weaponNum - 1]->GetComponent<Script_Weapon>()->GetWeaponType()) {
	case WeaponType::HandedGun:
		weaponIdx = 1;
		break;
	case WeaponType::AssaultRifle:
		weaponIdx = 2;
		break;
	case WeaponType::MineLauncher:
		weaponIdx = 3;
		break;
	case WeaponType::ShotGun:
		weaponIdx = 5;
		break;
	case WeaponType::MissileLauncher:
		weaponIdx = 6;
		break;
	case WeaponType::Sniper:
		weaponIdx = 7;
		break;
	default:
		assert(0);
		break;
	}

	mController->SetValue("Weapon", weaponIdx);

	// synchronize animation frame
	// pistol's animation is different. so can't synchronize with others
	if (isDrawImmed && weaponNum != kPistolNum && GetCrntWeaponNum() != kPistolNum) {
		mController->SetValue("Draw", true, true);
		auto motion = mController->GetCrntMotion("Body");
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

	if (mWeaponScript) {
		auto motion = mReloadMotions[static_cast<int>(mWeaponScript->GetWeaponName())];
		SetMotionSpeed(motion, mWeaponScript->GetReloadTime());
	}
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
	mCrntYawAngle = 0;

	if (mWeapon) {
		StopReload();
		mController->SetValue("PutBack", true);
	}
}

void Script_GroundPlayer::PutbackWeaponEndCallback()
{
	mController->SetValue("Weapon", 0);

	base::PutbackWeaponEnd();

	// If you don't have the next weapon, end the aiming state.
	if (GetNextWeaponNum() == -1 && mIsAim) {
		OffAim();
	}
	mController->SetValue("PutBack", false);
}


void Script_GroundPlayer::UpdateParam(float val, float& param)
{
	constexpr float kParamChangeSpeed   = 6.f;
	constexpr float kOppositeExtraSpeed = 8.f;		// Additional movement speed when moving to the opposite side

	int sign = Math::Sign(val);						// sign = Direction of param = Sign of the current input value
	if (Math::IsZero(val)) {						//		  If there is no input, the opposite sign of the current parameter
		if (Math::IsZero(param)) {
			return;
		}
		sign = -Math::Sign(param);
	}
	float before = param;
	param += (kParamChangeSpeed * sign) * DeltaTime();

	if (!Math::IsZero(val)) {
		if (fabs(param) < 0.5f && (fabs(before) > fabs(param))) {	// When moving to the opposite side
			param += (sign * kOppositeExtraSpeed) * DeltaTime();	// apply opposite extra speed
		}
		else if (fabs(param) >= fabs(before)) {						// When moving forward
			param = std::clamp(param, -fabs(val), fabs(val));		// Do not allow the param to cross the val.

			// If the inc/dec width is low && it is close to 0, the param is ignored.
			if ((fabs(fabs(param) - fabs(before)) < 0.001f) && (fabs(param) < 0.1f)) {
				param = 0.f;
			}
		}
	}

	param = std::clamp(param, -1.f, 1.f);		// Fixed between -1 ~ 1
}

void Script_GroundPlayer::ResetWeaponAnimation()
{
	mController->SetValue("Weapon", 0);
	mController->SetValue("PutBack", false);
	mController->SetValue("Aim", false);
	mController->SetValue("Reload", false);
	mController->SetValue("BoltAction", false);
	mController->SetValue("Draw", false);
}

void Script_GroundPlayer::UpdateMovement(Dir dir)
{
	// The current character's movement state is set according to the key input. //
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
	// center of rotation
	const Vec3 offsetPos = mSpineBone->GetPosition().xz();

	// Positon & direction of the muzzle
	const Vec3 muzzlePos = mMuzzle->GetPosition().xz();
	const Vec3 muzzleLook = mMuzzle->GetLook().xz();

	// Vector from center of rotation to point of aim
	const Vec3 offsetToAim = aimWorldPos - offsetPos;

	// Length approximation of the vector from the muzzle to the impact point
	const float approxLength = (offsetToAim.Length() - (muzzlePos - offsetPos).Length());

	// Vector to the impact point
	const Vec3 muzzleToImpact = muzzlePos + (muzzleLook * approxLength);

	// Vector from the center of rotation to the point of impact
	const Vec3 offsetToBullet = muzzleToImpact - offsetPos;

	// Angle (center of rotation->impact point) -> (center of rotation->aim point)
	return Vector3::SignedAngle(offsetToBullet, offsetToAim, Vector3::Up);
}

float Script_GroundPlayer::GetAngleSpineToAim(const Vec3& aimWorldPos) const
{
	return Vector3::SignedAngle(mSpineBone->GetUp().xz(), aimWorldPos.xz() - mSpineBone->GetPosition().xz(), Vector3::Up);
}

Vec3 Script_GroundPlayer::GetAimWorldPos(const Vec2& aimScreenPos) const
{
	// Find the point that matches the y-value of the [muzzle] in the ray fired from the [aimScreenPos].
	const Vec3 ray = MAIN_CAMERA->ScreenToWorldRay(aimScreenPos);
	const Vec3 camPos = MAIN_CAMERA->GetPosition();
	return Vector3::RayOnPoint(camPos, ray, mMuzzle->GetPosition().y).xz();
}

void Script_GroundPlayer::RotateToAim(Dir dir, float& rotAngle)
{
	constexpr float kStopRotAngle = 10.f;
	const Vec2 aimDir = InputMgr::I->GetMouseDir();

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

	// If it's below the [kMaxAngle], interpolate
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
	mSpineBone->Rotate(Vector3::Forward, mCrntYawAngle);

	if (mIsAim && mMuzzle) {
		if (IsInGunChangeMotion()) {
			return;
		}

		// angle could be too large if aim is so close
		constexpr float kAimMinDistance = 100.f;
		Vec2 aimScreenPos = InputMgr::I->GetMousePos();
		if (aimScreenPos.Length() < kAimMinDistance) {
			aimScreenPos = Vector2::Normalized(aimScreenPos) * kAimMinDistance;
		}

		// smoothly rotate the spin angle through linear interpolation.
		::IncreaseDelta(mAimingDeltaTime, kAimingSpeed);

		const Vec3 aimWorldPos = GetAimWorldPos(aimScreenPos);
		// If it is reloading, the angle of rotation is calculated according to the direction of the spine
		if (IsReloading()) {
			// Maintain the current angle and slowly rotate until [mReloadingDeltaTime] becomes 1.
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
		else {
			// If you were reloading, keep your current angle and slowly rotate to your target point.
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
			
		// Remove the yaw rotation of the muzzle by rotating up and down.
		if (!mWeaponScript->IsReloading() && mController->IsEndTransition("Body")) {
			float yawAngle = -Vector3::SignedAngle(mMuzzle->GetLook(), mMuzzle->GetLook().xz(), mMuzzle->GetRight());
			if (fabs(yawAngle) > 0.1f) {
				// Maximum speed when above the [kMaxAngle]
				constexpr float kAlignSpeed  = 80.f;
				constexpr float kMaxAngle    = 3.f;
				constexpr float kMaxYawAngle = 10.f;
				const float ratio            = std::clamp(fabs(yawAngle) / kMaxAngle, 0.f, 1.f);
				const float speed            = kAlignSpeed * ratio;

				mCrntYawAngle += Math::Sign(yawAngle) * speed * DeltaTime();
				mCrntYawAngle = std::clamp(mCrntYawAngle, -kMaxYawAngle, kMaxYawAngle);
			}
		}

		// apply recoil
		if (mCurRecoil > 0.f) {
			mSpineBone->Rotate(Vector3::Forward, mCurRecoil);
		}
	}
	else if (::DecreaseDelta(mAimingDeltaTime, kHoldingSpeed)) {
		mSpineBone->RotateGlobal(Vector3::Up, mCrntSpineAngle * mAimingDeltaTime);
		mCrntSpineAngle *= mAimingDeltaTime;
	}
}

bool Script_GroundPlayer::Reload()
{
	if (!base::Reload()) {
		return false;
	}

	StartReload();
	
	return true;
}

void Script_GroundPlayer::StopReload()
{
	if (mController) {
		mController->SetValue("Reload", false);
	}
}

void Script_GroundPlayer::SetState(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState)
{
	// Only update the value if it is different from the previous state of motion.
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
	if (mWeaponScript) {
		mWeaponScript->StopReload();
	}
}

void Script_GroundPlayer::EndReloadCallback()
{
	EndReload();
}

void Script_GroundPlayer::BoltActionCallback()
{
	if (mController) {
		mController->SetValue("BoltAction", false);

		if (mWeapon) {
			mWeapon->DetachParent();
			Transform* rightHand = mObject->FindFrame("RefPosSniper_Action");
			rightHand->SetChild(mWeapon->GetShared());
			mWeapon->SetLocalTransform(Matrix::Identity);
		}
	}
}

void Script_GroundPlayer::BoltActionSoundCallback()
{
	SoundMgr::I->Play("Reload", "PipeLine BoltAction");
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
	if (!mCamera) {
		return;
	}

	// In the aiming state, set [offset_t] larger as the mouse's position approaches the boundary. (Max = 1)
	if (mIsAim) {
		const Vec2 mousePos = InputMgr::I->GetMousePos() * 2.f;
		const Vec2 ndc      = MAIN_CAMERA->ScreenToNDC(mousePos);
		const Vec2 ndcAbs   = Vec2(fabs(ndc.x), fabs(ndc.y));

		constexpr float offsetMaxRatio = 0.8f;
		float maxOffset_t = (std::max)(ndcAbs.x, ndcAbs.y);
		if (maxOffset_t > offsetMaxRatio) {
			maxOffset_t = offsetMaxRatio;
		}
		maxOffset_t /= offsetMaxRatio;

		mCamera->Move(mousePos, ndcAbs, maxOffset_t);
	}
	// When in the moving state, apply offset only up to [maxOffset_t]% depending on the direction
	else if(dir != Dir::None) {
		constexpr float maxOffset_t = 0.6f;
		const Vec3 dirVec = Transform::GetWorldDirection(dir);

		mCamera->Move(Vec2(dirVec.x, dirVec.z), Vector2::One, maxOffset_t);
	}
}

void Script_GroundPlayer::SetMotionSpeed(AnimatorMotion* motion, float time)
{
	if (time <= 0.f) {
		return;
	}

	const float motionSpeed = motion->GetMaxLength() / time;
	motion->ResetOriginSpeed(motionSpeed);
}
void Script_GroundPlayer::ComputeSlideVector(Object& other)
{
	//// 이전 충돌체와 이전 슬라이딩 벡터를 저장
	//static Object* prevOther = nullptr;
	//static Vec3 prevSlideVec{};

	// 무릎부터 이동 방향을 향하는 광선
	Ray ray{ mObject->GetPosition() + mObject->GetUp() * 0.25f, Vector3::Normalized(mDirVec) };

	//// 이전 충돌체가 현재 충돌체와 다른 경우
	//if (prevOther != nullptr) {
	//	if (prevOther->GetID() != other.GetID()) {
	//		// 광선으로부터 두 충돌체의 거리를 계산
	//		float crntDist = Vec3::Distance(ray.Position, other.GetPosition());
	//		float prevDist = Vec3::Distance(ray.Position, prevOther->GetPosition());

	//		// 현재 충돌체까지의 거리가 더 길 경우 이전 슬라이딩 벡터를 사용
	//		if (crntDist > prevDist) {
	//			mSlideVec = prevSlideVec;
	//			return;
	//		}
	//	}
	//}

	float dist{};
	float minDist{ 999.f };

	// 최소 거리가 작을 경우에만 실행
	sptr<Collider> box{};
	for (const auto& collider : other.GetComponent<ObjectCollider>()->GetColliders()) {
		if (collider->GetType() != Collider::Type::Box) {
			continue;
		}

		if (collider->Intersects(ray, dist)) {
			if (dist < minDist) {
				minDist = dist;
				box = collider;
			}
		}
	}

	if (box) {
		const auto& obb = reinterpret_cast<BoxCollider*>(box.get())->mBox;

		Matrix worldToOBB = Matrix::CreateFromQuaternion(obb.Orientation);

		ray.Position -= obb.Center;
		ray.Position = Vec3::Transform(ray.Position, worldToOBB.Invert());
		ray.Direction = Vec3::Transform(ray.Direction, worldToOBB.Invert());

		Vec3 collisionNormal;
		if (ray.Position.z <= obb.Extents.z && ray.Position.z >= -obb.Extents.z) {
			if (ray.Position.x >= 0.f) {
				collisionNormal = Vector3::Right;
			}
			else {
				collisionNormal = Vector3::Left;
			}
		}
		else {
			if (ray.Position.z >= 0.f) {
				collisionNormal = Vector3::Forward;
			}
			else {
				collisionNormal = Vector3::Backward;
			}
		}

		float rdn = ray.Direction.Dot(collisionNormal);
		if (rdn < 0.f) {
			mSlideVec = XMVector3Normalize(ray.Direction - collisionNormal * rdn);
			mSlideVec = Vec3::Transform(mSlideVec, worldToOBB);
		}
	}
}


void Script_GroundPlayer::ResetBoltActionMotionSpeed(rsptr<Script_Weapon> weapon)
{
	if (weapon->GetWeaponType() != WeaponType::Sniper) {
		return;
	}

	auto boltActionMotion = mController->FindMotionByName("BoltActionSniper", "Body");

	// motion speed
	constexpr float decTime = 0.1f;
	const float fireDelay = weapon->GetFireDelay();
	SetMotionSpeed(boltActionMotion, fireDelay - decTime);
}

void Script_GroundPlayer::SwitchWeapon(GridObject* weapon)
{
	int weaponIdx = Script_Weapon::GetWeaponIdx(weapon->GetComponent<Script_Weapon>(true)->GetWeaponType());

	// drop & switch weapons //
	int crntWeaponIdx = GetCrntWeaponIdx();
	if (mWeapons[weaponIdx]) {
		DropWeapon(weaponIdx);
	}

	mWeapons[weaponIdx] = weapon;
	if (crntWeaponIdx == weaponIdx || !mWeapon) {
		DrawWeapon(weaponIdx + 1);
	}
}

void Script_GroundPlayer::SetWeaponChild(GridObject* weapon)
{
	const auto& weaponScript = weapon->GetComponent<Script_Weapon>(true);
	WeaponType weaponType = weaponScript->GetWeaponType();

	Transform* transform = mObject->FindFrame(kDefaultTransforms.at(weaponType), true);
	transform->SetChild(weapon->GetShared());
	weapon->SetLocalTransform(Matrix::Identity);
}
