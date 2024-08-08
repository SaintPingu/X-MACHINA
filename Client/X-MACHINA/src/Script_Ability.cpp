#include "stdafx.h"
#include "Script_Ability.h"

#include "BattleScene.h"
#include "Shader.h"
#include "Object.h"
#include "Mesh.h"
#include "Timer.h"

#include "Script_AbilityManager.h"

void Script_Ability::Update()
{
	base::Update();

	(this->*mUpdateFunc)();
}

void Script_Ability::OnEnable()
{
	if (mState != State::Ready) {
		return;
	}

	base::OnEnable();

	On();
}

void Script_Ability::OnDisable()
{
	if (mState == State::Active) {
		if (mType == Type::Cooldown && mCurCooldownTime > 0.f) {
			return;
		}
		Off();
	}
	else if(mCurCooldownTime <= 0.f || mState == State::Ready) {
		base::OnDisable();

		Ready();
	}
}


void Script_Ability::Toggle()
{
	switch (mState) {
	case State::Ready:
		SetActive(true);
		break;
	case State::Active:
		if (mType != Type::Cooldown) {
			SetActive(false);
		}
		break;
	case State::Cooldown:
		break;
	default:
		assert(0);
		break;
	}
}

void Script_Ability::Init(std::string abilityName, float cooldownTime, float activeTime)
{
	mAbilityName = abilityName;
	mMaxCooldownTime = cooldownTime;
	mMaxActiveTime = activeTime;
}

void Script_Ability::On()
{
	mState = State::Active;
	mCurActiveTime = mMaxActiveTime;
}

void Script_Ability::Off()
{
	mState = State::Cooldown;
	mCurCooldownTime = mMaxCooldownTime;
}

void Script_Ability::Ready()
{
	mState = State::Ready;
}


void Script_Ability::SetType(Type type)
{
	mType = type;

	switch (type) {
	case Type::Default:
	case Type::Cooldown:
		mUpdateFunc = &Script_Ability::UpdateDefault;
		break;
	case Type::Toggle:
		mUpdateFunc = &Script_Ability::UpdateToggle;
		break;
	default:
		assert(0);
		break;
	}
}

void Script_Ability::UpdateDefault()
{
	switch (mState)
	{
	case State::Ready:
		break;
	case State::Active:
	{
		mCurActiveTime -= DeltaTime();
		if (mCurActiveTime <= 0) {
			mCurActiveTime = 0;
			SetActive(false);
		}
	}
	break;
	case State::Cooldown:
	{
		mCurCooldownTime -= DeltaTime();
		if (mCurCooldownTime <= 0.f) {
			mCurCooldownTime = 0.f;
			SetActive(false);
		}
	}
	break;
	default:
		break;
	}
}

void Script_Ability::UpdateToggle()
{
	switch (mState)
	{
	case State::Ready:
		break;
	case State::Active:
		mCurActiveTime -= DeltaTime();
		break;
	case State::Cooldown:
	{
		mCurCooldownTime -= DeltaTime();
		if (mCurCooldownTime <= 0.f) {
			mCurCooldownTime = 0.f;
			SetActive(false);
		}
	}
		break;
	default:
		break;
	}
}




void Script_RenderedAbility::Update()
{
	base::Update();

	UpdateAbilityCB(mCurActiveTime);
}

void Script_RenderedAbility::On()
{
	base::On();

	BattleScene::I->GetManager()->GetComponent<Script_AbilityManager>()->AddRenderedAbilities(mLayer, this);
}

void Script_RenderedAbility::Off()
{
	base::Off();

	BattleScene::I->GetManager()->GetComponent<Script_AbilityManager>()->RemoveRenderedAbilities(mLayer, this);
}

void Script_RenderedAbility::Render()
{
	if (mShader) {
		mShader->Set();
	}

	if (mRenderedObject) {
		mRenderedObject->Render();
	}

	if (mRenderedMesh) {
		mRenderedMesh->Render();
	}
}


void Script_RenderedAbility::UpdateAbilityCB(float activeTime)
{
	mAbilityCB.ActiveTime = mMaxActiveTime;
	mAbilityCB.AccTime = mMaxActiveTime - mCurActiveTime;

	BattleScene::I->UpdateAbilityCB(mAbilityCBIdx, mAbilityCB);
}