#include "stdafx.h"
#include "Script_Ability_Cloaking.h"

#include "Component/ParticleSystem.h"
#include "Timer.h"

#include "Script_Player.h"
#include "Script_AfterImageObject.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"





/// +-------------------------------------------------
///		Script_Ability_Cloaking 
/// -------------------------------------------------+
void Script_Ability_Cloaking::Awake()
{
	base::Awake();

	Init("Cloaking", 3.f);
	SetPheroCost(30.f);
	SetIconUI("Icon_B_Cloaking", "Icon_F_Cloaking", Vec2{ 70.f, 100.f }, Vec2{ 50.f }, mMaxCooldownTime);

	mAfterImage = mObject->AddComponent<Script_AfterImageObject>();
	mAfterImage->SetAfterImage(10, 1.f);

	SetType(Type::Toggle);
}

void Script_Ability_Cloaking::Start()
{
	base::Start();

	mObject->mObjectCB.HitRimColor = Vec3{ 2.f, 1.f, 2.f };
	mPrevInvokerTag = mObject->GetTag();

	mPlayer = mObject->GetComponent<Script_PheroPlayer>();
}

void Script_Ability_Cloaking::Update()
{
	base::Update();

	UpdateCooldownBarUI(mMaxCooldownTime, mCurCooldownTime);

	if (!ReducePheroAmount()) {
		SetActive(false);
	}
}

void Script_Ability_Cloaking::On()
{
	base::On();
	mBuffPS = ParticleManager::I->Play("MagicCircle_Dot", mObject);

	mObject->mObjectCB.HitRimFactor = 1.f;
	mAfterImage->SetActiveUpdate(true);
	mObject->SetTag(ObjectTag::AfterSkinImage);

#ifdef SERVER_COMMUNICATION
	/// +-------------------------------
	///		SKILLPACKET BROADCAST
	/// -------------------------------+
	auto cpkt = FBS_FACTORY->CPkt_Player_OnSkill(FBProtocol::PLAYER_SKILL_TYPE_CLOACKING);
	CLIENT_NETWORK->Send(cpkt);
#endif
}

void Script_Ability_Cloaking::Off()
{
	base::Off();
	if (mBuffPS) {
		mBuffPS->Stop();
	}

	mObject->mObjectCB.HitRimFactor = 0.f;
	mAfterImage->SetActiveUpdate(false);
	mObject->SetTag(mPrevInvokerTag);
}

bool Script_Ability_Cloaking::ReducePheroAmount(bool checkOnly)
{
	if (mPlayer) {
		return mPlayer->ReducePheroAmount(mPheroCost * DeltaTime());
	}

	return false;
}






/// +-------------------------------------------------
///		Script_Remote_Ability_Cloaking 
/// -------------------------------------------------+
void Script_Remote_Ability_Cloaking::Update()
{
	Script_RenderedAbility::Update();
}

void Script_Remote_Ability_Cloaking::On()
{
	Script_RenderedAbility::On();
	mBuffPS = ParticleManager::I->Play("MagicCircle_Dot", mObject);
	mObject->mObjectCB.HitRimFactor = 1.f;
	mAfterImage->SetActiveUpdate(true);
	mObject->SetTag(ObjectTag::AfterSkinImage);
}

void Script_Remote_Ability_Cloaking::Off()
{
	Script_RenderedAbility::Off();
	if (mBuffPS) {
		mBuffPS->Stop();
	}

	mObject->mObjectCB.HitRimFactor = 0.f;
	mAfterImage->SetActiveUpdate(false);
	mObject->SetTag(mPrevInvokerTag);
}
