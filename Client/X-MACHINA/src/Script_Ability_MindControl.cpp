#include "stdafx.h"
#include "Script_Ability_MindControl.h"

#include "Component/Camera.h"
#include "ResourceMgr.h"
#include "Object.h"
#include "Texture.h"
#include "Component/Collider.h"

#include "Script_Player.h"
#include "Script_AimController.h"
#include "Script_DefaultEnemyBT.h"
#include "Script_MindControlledEnemyBT.h"
#include "Script_BehaviorTree.h"
#include "GameFramework.h"

#include "ClientNetwork/Contents/Script_NetworkEnemy.h"
#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"


void Script_Ability_MindControl::Awake()
{
	base::Awake();
	Init("MindControl", 10.f, 30.f);
	SetPheroCost(200.f);

	mMaxControlledObjectCnt = 1;
	mCurrControlledObjectCnt = mMaxControlledObjectCnt;
	mMindControlAimTexture = RESOURCE<Texture>("MindControlAim");
	SetType(Type::Cooldown);
}

void Script_Ability_MindControl::Start()
{
	base::Start();

	mPlayer = mObject->GetComponent<Script_PheroPlayer>();
}

void Script_Ability_MindControl::Update()
{
	base::Update();
}

bool Script_Ability_MindControl::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (messageID == WM_LBUTTONDOWN) {
		Click();
	}

	return true;
}

void Script_Ability_MindControl::On()
{
	if (!ReducePheroAmount(true)) {
		SetActive(false);
		return;
	}

	base::On();

	mCurrControlledObjectCnt = mMaxControlledObjectCnt;

	mAimController = mObject->GetComponent<Script_AimController>();
	if (!mAimController) {
		SetActive(false);
		return;
	}

	ChangeAimToActive();
}

void Script_Ability_MindControl::Off()
{
	base::Off();

	ChangeAimToOrigin();

	mPickedTarget = nullptr;
}

bool Script_Ability_MindControl::ReducePheroAmount(bool checkOnly)
{
	if (mPlayer) {
		return mPlayer->ReducePheroAmount(mPheroCost, checkOnly);
	}

	return false;
}

Object* Script_Ability_MindControl::PickingObject(const Vec2& screenPos)
{
	float minDistance = FLT_MAX;
	Object* pickedObject = nullptr;

	// 주변 그리드 내 모든 적 객체
	for (const auto& [id, enemyScript] : CLIENT_NETWORK->GetRemoteMonsters()) {
		const Vec3 dir = MAIN_CAMERA->ScreenToWorldRay(InputMgr::I->GetMousePos());
		const Vec3 pos = MAIN_CAMERA->GetPosition();
		Ray ray{ pos, dir };
		ray.Direction.Normalize();

		// 월드 좌표계에서 레이와 바운딩 스피어의 충돌 검사
		float distance = 0.f;
		GridObject* enemy = static_cast<GridObject*>(enemyScript->GetObj());
		if (ray.Intersects(enemy->GetCollider()->GetBS(), distance) == false)
			continue;

		// 거리가 가장 짧은 객체 픽킹 선별
		if (distance < minDistance) {
			minDistance = distance;
			pickedObject = enemy;
		}
	}

	return pickedObject;
}

void Script_Ability_MindControl::Click()
{
	if (mCurrControlledObjectCnt <= 0) {
		return;
	}

	mPickedTarget = PickingObject(InputMgr::I->GetMousePos());

	if (mPickedTarget) {
		if (!ReducePheroAmount()) {
			SetActive(false);
			return;
		}

		auto cpkt = FBS_FACTORY->CPkt_Player_OnSkill(FBProtocol::PLAYER_SKILL_TYPE_MIND_CONTROL, mPickedTarget->GetID());
		CLIENT_NETWORK->Send(cpkt);

		mPickedTarget->mObjectCB.MindRimFactor = 1.f;

		mCurrControlledObjectCnt--;
	}

	if (mCurrControlledObjectCnt <= 0) {
		ChangeAimToOrigin();
	}
}

void Script_Ability_MindControl::ChangeAimToOrigin()
{
	mAimController->ChangeAimTexture(mPrevAimTexture, mPrevAimScale);
}

void Script_Ability_MindControl::ChangeAimToActive()
{
	mPrevAimTexture = mAimController->GetTexture();
	mPrevAimScale = mAimController->GetTextureScale();
	mAimController->ChangeAimTexture(mMindControlAimTexture, Vec2(300.f, 300.f));
}

void Script_Remote_Ability_MindControl::SetPickingObject(Object* target)
{
	if (target) {
		mPickedTarget = target;
	}
}

void Script_Remote_Ability_MindControl::On()
{
	Script_RenderedAbility::On();
	mPickedTarget->mObjectCB.MindRimFactor = 1.f;
}

void Script_Remote_Ability_MindControl::Off()
{
	Script_RenderedAbility::Off();
	mPickedTarget->mObjectCB.MindRimFactor = 0.f;
	mPickedTarget = nullptr;
}
