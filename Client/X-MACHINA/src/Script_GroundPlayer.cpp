#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Bullet.h"
#include "Script_AimController.h"
#include "Script_MainCamera.h"
#include "Script_Weapon.h"
#include "Script_Weapon_Pistol.h"
#include "Script_Weapon_Rifle.h"
#include "Script_Weapon_Shotgun.h"
#include "Script_Weapon_Sniper.h"
#include "Script_Weapon_MissileLauncher.h"
#include "Script_AbilityHolder.h"
#include "Script_Item.h"

#include "Component/Rigidbody.h"
#include "Component/Camera.h"
#include "Component/Collider.h"

#include "BattleScene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "InputMgr.h"
#include "Timer.h"
#include "Animator.h"
#include "AnimationClip.h"
#include "AnimatorMotion.h"
#include "AnimatorController.h"

#include "Component/UI.h"

#include "ShieldAbility.h"
#include "IRDetectorAbility.h"
#include "MinimapAbility.h"
#include "MindControlAbility.h"
#include "CloakingAbility.h"


#pragma region Variable
const float Script_GroundPlayer::mkSitWalkSpeed   = 1.5f;
const float Script_GroundPlayer::mkStandWalkSpeed = 2.2f;
const float Script_GroundPlayer::mkRunSpeed       = 5.f;
const float Script_GroundPlayer::mkSprintSpeed    = 8.f;

const float Script_GroundPlayer::mkStartRotAngle = 40.f;

namespace {
	constexpr int kPistolNum = 1;
	constexpr int kDrawFrame = 13;	// the hand is over the shoulder

	static const std::unordered_map<WeaponName, WeaponType> kWeaponTypes{
		{WeaponName::H_Lock, WeaponType::HandedGun },
		{WeaponName::DBMS, WeaponType::ShotGun },
		{WeaponName::SkyLine, WeaponType::AssaultRifle },
		{WeaponName::Burnout, WeaponType::MissileLauncher },
		{WeaponName::PipeLine, WeaponType::Sniper },
	};

	static const std::unordered_map<WeaponType, std::string> kDefaultTransforms{
		{WeaponType::HandedGun, "RefPos2HandedGun_Action" },
		{WeaponType::AssaultRifle, "RefPosAssaultRifle_Action" },
		{WeaponType::ShotGun, "RefPosShotgun_Action" },
		{WeaponType::MissileLauncher, "RefPosMissileLauncher_Action" },
		{WeaponType::Sniper, "RefPosSniper_Action" },
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
	mObject->AddComponent<Script_AbilityHolder>()->SetAbility('T', std::make_shared<ShieldAbility>());
	mObject->AddComponent<Script_AbilityHolder>()->SetAbility('Y', std::make_shared<IRDetectorAbility>());
	mObject->AddComponent<Script_AbilityHolder>()->SetAbility('U', std::make_shared<MindControlAbility>());
	mObject->AddComponent<Script_ToggleAbilityHolder>()->SetAbility('I', std::make_shared<CloakingAbility>());
	mObject->AddComponent<Script_ToggleAbilityHolder>()->SetAbility(VK_TAB, std::make_shared<MinimapAbility>());

	// values //
	mSpineBone = mObject->FindFrame("Humanoid_ Spine1");
	const auto& aimUI = Canvas::I->CreateUI<UI>(3, "Aim", Vec2::Zero, Vec2(30, 30));
	mAimController = mObject->AddComponent<Script_AimController>();
	mAimController->SetUI(aimUI);
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

	constexpr Vec3 kSpawnPoint = Vec3(73, 0, 230);

	SetSpawn(kSpawnPoint);
	mObject->SetPosition(kSpawnPoint);
}


void Script_GroundPlayer::Update()
{
	base::Update();

	mIsInteracted = false;

	ProcessInput();

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
		ComputeSlideVector(other);
		break;
	default:
		break;
	}
}


void Script_GroundPlayer::UpdateParams(Dir dir, float v, float h, float rotAngle)
{
	if (mIsAim) {										// ���� ���¶��??, �÷��̾��� look���⿡ ���� �ٸ� �ٸ�(Legs) �ִϸ��̼��� ����Ѵ�??.

		const float rotAngleAbs = fabs(rotAngle);
		// �̵� ���� //
		if (!Math::IsZero(v) || !Math::IsZero(h)) {
			const Vec3 movementDir = Transform::GetWorldDirection(dir);

			const float lookAngle = Vector3::SignedAngle(mObject->GetLook().xz(), Vector3::Forward, Vector3::Up);
			Vec3 rotatedMovementDir = Vector3::Normalized(Vector3::Rotate(movementDir, Vector3::Up, lookAngle));

			// v, h�� ����, dir�� ũ�Ⱑ �׻� ���簢�� ������ �굵�� �Ѵ�.
			// BlendTree�� �� ���� ���簢���� ���� ��ġ�� �ֱ� �����̴�.
			// �Ʒ� �����?? ���� ������ ����Ȯ�� v, h���� ��������. (���� ���ڿ�������)
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
			v = Math::Sign(rotAngle) > 0 ? 0.5f : -0.5f;
			h = Math::Sign(rotAngle) > 0 ? -1.f : 1.f;
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


bool Script_GroundPlayer::ProcessInput()
{
	if (!base::ProcessInput()) {
		// TODO : �÷��̾� ���?? �ִϸ��̼� ����
		return false;
	}

	// Ű �Է¿� ���� ���� ���� //
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

	default:
		break;
	}

	return true;
}

bool Script_GroundPlayer::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!base::ProcessKeyboardMsg(messageID, wParam, lParam)) {
		return false;
	}

	switch (messageID) {
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		{
			const int weaponNum = static_cast<int>(wParam - '0');
			DrawWeapon(weaponNum);
		}
			break;
		case '6':
			AquireNewWeapon(WeaponName::SkyLine);
			break;
		case '7':
			AquireNewWeapon(WeaponName::DBMS);
			break;
		case '8':
			AquireNewWeapon(WeaponName::Burnout);
			break;
		case '9':
			AquireNewWeapon(WeaponName::PipeLine);
			break;
		case 'E':
			Interact();
			break;
		case 'G':
			DropWeapon(GetCrntWeaponNum() - 1);
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

	return true;
}

void Script_GroundPlayer::InitWeaponAnimations()
{
	static const std::unordered_map<WeaponType, std::string> kReloadMotions{
		{WeaponType::HandedGun, "Reload2HandedGun" },
		{WeaponType::AssaultRifle, "ReloadAssaultRifle" },
		{WeaponType::ShotGun, "ReloadShotgun" },
		{WeaponType::MissileLauncher, "ReloadMissileLauncher"  },
		{WeaponType::Sniper, "ReloadAssaultRifle" },
	};

	static const std::unordered_map<WeaponType, std::string> kDrawMotions{
		{WeaponType::HandedGun, "Draw2HandedGun" },
		{WeaponType::AssaultRifle, "DrawAssaultRifle" },
		{WeaponType::ShotGun, "DrawShotgun" },
		{WeaponType::MissileLauncher, "DrawMissileLauncher" },
		{WeaponType::Sniper, "DrawAssaultRifle" },
	};

	static const std::unordered_map<WeaponType, std::string> kPutbackMotions{
		{WeaponType::HandedGun, "PutBack2HandedGun" },
		{WeaponType::AssaultRifle, "PutBackAssaultRifle" },
		{WeaponType::ShotGun, "PutBackShotgun" },
		{WeaponType::MissileLauncher, "PutBackMissileLauncher" },
		{WeaponType::Sniper, "PutBackAssaultRifle" },
	};

	// �ִϸ��̼� �ݹ� �Լ� //
	const std::function<void()> reloadCallback = std::bind(&Script_GroundPlayer::EndReloadCallback, this);
	const std::function<void()> reloadStopCallback = std::bind(&Script_GroundPlayer::StopReloadCallback, this);
	const std::function<void()> reloadChangeCallback = std::bind(&Script_GroundPlayer::ChangeReloadCallback, this);
	const std::function<void()> drawCallback = std::bind(&Script_GroundPlayer::DrawWeaponCallback, this);
	const std::function<void()> drawEndCallback = std::bind(&Script_GroundPlayer::DrawWeaponEndCallback, this);
	const std::function<void()> putbackCallback = std::bind(&Script_GroundPlayer::PutbackWeaponEndCallback, this);


	// bolt action sniper �ʱ�ȭ
	{
		const std::function<void()> boltActionCallback = std::bind(&Script_GroundPlayer::BoltActionCallback, this);
		const auto& boltActionMotion = mController->FindMotionByName("BoltActionSniper", "Body");

		// callback
		boltActionMotion->AddEndCallback(boltActionCallback);
		boltActionMotion->AddChangeCallback(boltActionCallback);
	}

	

	// setting callbacks //
	constexpr int kPutbackFrame = 15;	// the hand is over the shoulder

	for (int i = 0; i < static_cast<int>(WeaponName::_count) - 1; ++i) {
		WeaponType weaponType = static_cast<WeaponType>(i);

		const auto& realodMotion = mReloadMotions[static_cast<int>(weaponType)] = mController->FindMotionByName(kReloadMotions.at(weaponType), "Body");
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
}

void Script_GroundPlayer::BulletFired()
{
	constexpr float recoilAmount = 5.f;
	mCurRecoil += recoilAmount;
	if (fabs(mCurRecoil) >= mMaxRecoil) {
		mCurRecoil = mMaxRecoil;
	}
}



void Script_GroundPlayer::AquireNewWeapon(WeaponName weaponName)
{
	if (weaponName == WeaponName::None) {
		return;
	}

	GameObject* weapon = BattleScene::I->Instantiate(Script_Weapon::GetWeaponModelName(weaponName), ObjectTag::Dynamic, ObjectLayer::Default, false);

	// ��ũ��Ʈ �߰� //
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
	default:
		assert(0);
		break;
	}

	// ��ũ��Ʈ ���� //
	weapon->GetComponent<Script_Weapon>()->SetOwner(this);

	SwitchWeapon(weapon);
	SetWeaponChild(weapon);
}

void Script_GroundPlayer::TakeWeapon(rsptr<Script_Weapon> weapon)
{
	GameObject* gameObject = weapon->GetObj()->GetObj<GameObject>();
	SwitchWeapon(gameObject);
	SetWeaponChild(gameObject);
	gameObject->GetComponent<Script_Weapon>()->SetOwner(this);

	if (weapon->GetWeaponType() == WeaponType::Sniper) {
		ResetBoltActionMotionSpeed(weapon);
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
	case WeaponType::ShotGun:
		weaponIdx = 5;
		break;
	case WeaponType::MissileLauncher:
		weaponIdx = 6;
		break;
	case WeaponType::Sniper:
		weaponIdx = 7;
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
		auto motion = mReloadMotions[static_cast<int>(mWeaponScript->GetWeaponType())];
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

	// ���� ���Ⱑ ���ٸ� ���� ���¸� �����Ѵ�.
	if (GetNextWeaponNum() == -1 && mIsAim) {
		OffAim();
	}
	mController->SetValue("PutBack", false);
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

void Script_GroundPlayer::UpdateParam(float val, float& param)
{
	constexpr float kParamSpeed         = 6.f;		// �Ķ����?? ��ȯ �ӵ�
	constexpr float kOppositeExtraSpeed = 8.f;		// �ݴ��� �̵� �� �߰� �̵� ��ȯ �ӵ�

	int sign = Math::Sign(val);						// sign : �Ķ����?? �̵� ���� = ���� �Է°��� ��ȣ
	if (Math::IsZero(val)) {						//		  �Է��� ���ٸ� ���� �Ķ������?? �ݴ� ��ȣ
		if (Math::IsZero(param)) {
			return;
		}
		sign = -Math::Sign(param);
	}
	float before = param;
	param += (kParamSpeed * sign) * DeltaTime();	// �Ķ���Ͱ�?? ����

	if (!Math::IsZero(val)) {
		if (fabs(param) < 0.5f && (fabs(before) > fabs(param))) {	// �ݴ��� �̵� ��
			param += (sign * kOppositeExtraSpeed) * DeltaTime();	// �߰� ��ȯ �ӵ� ����
		}
		else if (fabs(param) >= fabs(before)) {						// ������ �̵� ��
			param = std::clamp(param, -fabs(val), fabs(val));		// param�� val�� ���� ���ϵ��� �Ѵ�.

			// �������� ���� && 0�� �����?? ���?? �ش� �Ķ���ʹ�?? �����Ѵ�.
			if (fabs(fabs(param) - fabs(before)) < 0.001f && fabs(param) < 0.1f) {								// 0�� �����ϸ� 0���� ����
				param = 0.f;
			}
		}
	}

	param = std::clamp(param, -1.f, 1.f);		// -1 ~ 1 ���̷� ����
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
	// ���� ĳ������ ������ ���¸� Ű �Է¿� ���� �����Ѵ�.
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
	// ȸ����
	const Vec3 offsetPos = mSpineBone->GetPosition().xz();

	// �ѱ� ��ġ&����
	const Vec3 muzzlePos = mMuzzle->GetPosition().xz();
	const Vec3 muzzleLook = mMuzzle->GetLook().xz();

	// ȸ���࿡�� ���������� ���ϴ� ����
	const Vec3 offsetToAim = aimWorldPos - offsetPos;

	// �ѱ����� ź�������� ���ϴ� ������ ���� �ٻ簪
	const float approxLength = (offsetToAim.Length() - (muzzlePos - offsetPos).Length());

	// ź�������� ���ϴ� ����
	const Vec3 muzzleToBullet = muzzlePos + (muzzleLook * approxLength);

	// ȸ���࿡�� ź�������� ���ϴ� ����
	const Vec3 offsetToBullet = muzzleToBullet - offsetPos;

	// (ȸ����-ź����) -> (ȸ����-������)���� ���ϴ� ����
	return Vector3::SignedAngle(offsetToBullet, offsetToAim, Vector3::Up);
}

float Script_GroundPlayer::GetAngleSpineToAim(const Vec3& aimWorldPos) const
{
	return Vector3::SignedAngle(mSpineBone->GetUp().xz(), aimWorldPos.xz() - mSpineBone->GetPosition().xz(), Vector3::Up);
}

Vec3 Script_GroundPlayer::GetAimWorldPos(const Vec2& aimScreenPos) const
{
	// aim���� �߻��?? �������� �ѱ��� y���� ��ġ�ϴ� ������ ã�´�.
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

	// [kMaxAngle] ���� ���ϸ� ����
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
		// ������ ���̶��??, �ѱ��� �ƴ� ô���� ���⿡ ���� ȸ������ ���Ѵ�.
		if (IsReloading()) {
			// ���� ������ �����ϸ� [mReloadingDeltaTime]�� 1�� �� �� ���� ������ ȸ���Ѵ�.
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
		// ������ ���̾��ٸ� ���� ������ �����ϸ� ��ǥ �������� ������ ȸ���Ѵ�.
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
			
		// ���ϸ� ȸ���Ͽ� �ѱ��� yaw ȸ���� �����Ѵ�.
		if (!mWeaponScript->IsReloading() && mController->IsEndTransition("Body")) {
			float yawAngle = -Vector3::SignedAngle(mMuzzle->GetLook(), mMuzzle->GetLook().xz(), mMuzzle->GetRight());
			if (fabs(yawAngle) > 0.1f) {
				// [maxAngle]�� �̻��� �� �ִ� �ӵ�
				constexpr float alignSpeed = 100.f;
				constexpr float maxAngle   = 3.f;
				const float ratio          = std::clamp(fabs(yawAngle) / maxAngle, 0.f, 1.f);
				const float speed          = alignSpeed * ratio;

				mCrntYawAngle += Math::Sign(yawAngle) * speed * DeltaTime();
			}
		}

		// �ݵ� ����
		if (mCurRecoil > 0.f) {
			mSpineBone->Rotate(Vector3::Forward, mCurRecoil);
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

	// ���� ���� ȸ�����¿��ٸ� �̸� ����Ѵ�??.
	if (mIsInBodyRotation) {
		mIsInBodyRotation = false;

		PlayerMotion prevMotion = mPrevMovement & 0xF0;
		if (prevMotion == PlayerMotion::None) {
			mController->SetValue("Walk", false);
		}
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
	// ���� ������ ���¿� �ٸ� ���?? ���� ������Ʈ �Ѵ�.
	// ���� ���¸� ����ϰ�?? ���� ���·� ��ȯ�Ѵ�.
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
	// ���� ������ �����?? ����ϰ�?? ���� ������ �������?? ��ȯ�Ѵ�.
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

	// ���ε� ���� ���?? ���� �� 80%�̻� ����Ǿ��ٸ�?? ���� �Ϸ� ó��
	// ���� ������ ������ ���?? ����
	constexpr float kAllowRatio = 0.8f;
	if (ratio > kAllowRatio && IsReloading()) {
		EndReload();
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
	// ���� �����̸�, ���콺�� ��ġ�� ���?? �������?? ���� [offset_t]�� ũ�� �����Ѵ�. (�ִ� 1)
	if (mIsAim) {
		const Vec2 mousePos = InputMgr::I->GetMousePos() * 2.f;
		const Vec2 ndc      = MAIN_CAMERA->ScreenToNDC(mousePos);
		const Vec2 ndcAbs   = Vec2(fabs(ndc.x), fabs(ndc.y));

		constexpr float offsetMaxRatio = 0.8f; // �ִ� [n]% ������ ���� (��ũ�� ������ [n - 100]% ���������� �ִ� offset ����)
		float maxOffset_t = (std::max)(ndcAbs.x, ndcAbs.y);
		if (maxOffset_t > offsetMaxRatio) {
			maxOffset_t = offsetMaxRatio;
		}
		maxOffset_t /= offsetMaxRatio;

		mCamera->Move(mousePos, ndcAbs, maxOffset_t);
	}
	// �̵� �����̸�, ���⿡ ���� offset�� �ִ� [maxOffset_t]%�� �����Ѵ�
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
	//// ���� �浹ü�� ���� �����̵� ���͸� ����
	//static Object* prevOther = nullptr;
	//static Vec3 prevSlideVec{};

	// �㸮 �ʺ��� �̵� ������ ���ϴ� ����
	Ray ray{ mObject->GetPosition() + mObject->GetUp() * 0.5f, Vector3::Normalized(mDirVec) };

	//// ���� �浹ü�� ���� �浹ü�� �ٸ� ���??
	//if (prevOther != nullptr) {
	//	if (prevOther->GetID() != other.GetID()) {
	//		// �������κ��� �� �浹ü�� �Ÿ��� ���??
	//		float crntDist = Vec3::Distance(ray.Position, other.GetPosition());
	//		float prevDist = Vec3::Distance(ray.Position, prevOther->GetPosition());

	//		// ���� �浹ü������ �Ÿ��� �� �� ���?? ���� �����̵� ���͸� ���??
	//		if (crntDist > prevDist) {
	//			mSlideVec = prevSlideVec;
	//			return;
	//		}
	//	}
	//}

	float dist{};
	float minDist{ 999.f };

	// �ּ� �Ÿ��� ���� ��쿡��?? ����
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

		//prevOther = &other;
		//prevSlideVec = mSlideVec;
	}
}

void Script_GroundPlayer::Interact()
{
	const auto collisionObjects = mObject->GetCollisionObjects();
	for (auto other : collisionObjects) {
		switch (other->GetTag()) {
		case ObjectTag::Crate:
		case ObjectTag::Item:
			if (!mIsInteracted) {
				 mIsInteracted = other->GetComponent<Script_Item>()->Interact(mObject);
			}
			break;
		default:
			break;
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
	constexpr float decTime = 0.1f; // [decTime]�� ��ŭ �ִϸ��̼��� �� ���� ����Ѵ�??.
	const float fireDelay = weapon->GetFireDelay();
	SetMotionSpeed(boltActionMotion, fireDelay - decTime);
}

void Script_GroundPlayer::SwitchWeapon(GameObject* weapon)
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

void Script_GroundPlayer::SetWeaponChild(GameObject* weapon)
{
	const auto& weaponScript = weapon->GetComponent<Script_Weapon>(true);

	// transform ���� //
	WeaponType weaponType = weaponScript->GetWeaponType();

	Transform* transform = mObject->FindFrame(kDefaultTransforms.at(weaponType), true);
	transform->SetChild(weapon->GetShared());
	weapon->SetLocalTransform(Matrix::Identity);
}
