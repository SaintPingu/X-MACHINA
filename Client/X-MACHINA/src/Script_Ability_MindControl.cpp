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

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"



void Script_Ability_MindControl::Awake()
{
	base::Awake();
	base::Init("MindControl", 10.f, 30.f);
	SetPheroCost(200.f);

	mMaxControlledObjectCnt = 1;
	mCurrControlledObjectCnt = mMaxControlledObjectCnt;
	mMindControlAimTexture = RESOURCE<Texture>("MindControlAim");
}

void Script_Ability_MindControl::Start()
{
	base::Start();

	mPlayer = mObject->GetComponent<Script_PheroPlayer>();
}

void Script_Ability_MindControl::Update()
{
	base::Update();

	if (KEY_TAP(VK_LBUTTON)) {
		Click();
	}
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

	if (mPickedTarget) {
		ActivePrevEnemyBT();
	}
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
	Matrix mtxProj = MAIN_CAMERA->GetProjMtx();

	float windowWidth = GameFramework::I->GetWindowSize().x;
	float windowHeight = GameFramework::I->GetWindowSize().y;

	// 스크린 좌표계를 투영 좌표계로
	float viewX = (2.f * screenPos.x / windowWidth - 1.f) / mtxProj(0, 0);
	float viewY = (-2.f * screenPos.y / windowHeight + 1.f) / mtxProj(1, 1);

	Matrix mtxView = MAIN_CAMERA->GetViewMtx();
	Matrix mtxViewInv = mtxView.Invert();

	float minDistance = FLT_MAX;
	Object* pickedObject = nullptr;

	// 주변 그리드 내 모든 적 객체
	const std::vector<sptr<Grid>>& grids = BattleScene::I->GetNeighborGrids(BattleScene::I->GetGridIndexFromPos(mObject->GetPosition()), true);
	for (const auto& grid : grids) {
		for (const auto& object : grid->GetObjectsFromTag(ObjectTag::Enemy)) {
			Ray ray(Vec3{ 0.f, 0.f, 0.f }, Vec3{ viewX, viewY, 1.f });

			// 뷰 좌표계를 월드 좌표계로
			ray.Position = XMVector3TransformCoord(ray.Position, mtxViewInv);
			ray.Direction = XMVector3TransformNormal(ray.Direction, mtxViewInv);
			ray.Direction.Normalize();

			// 월드 좌표계에서 레이와 바운딩 스피어의 충돌 검사
			float distance = 0.f;
			if (ray.Intersects(object->GetCollider()->GetBS(), distance) == false)
				continue;

			// 거리가 가장 짧은 객체 픽킹 선별
			if (distance < minDistance) {
				minDistance = distance;
				pickedObject = object;
			}
		}
	}

	return pickedObject;
}

void Script_Ability_MindControl::ActiveMindControlledEnemyBT()
{
	if (!mPickedTarget->GetComponent<Script_MindControlledEnemyBT>()) {
		mPickedTarget->AddComponent<Script_MindControlledEnemyBT>();
	}

	for (const auto& component : mPickedTarget->GetAllComponents()) {
		if (std::dynamic_pointer_cast<Script_BehaviorTree>(component)) {
			component->SetActive(false);
			break;
		}
	}

	auto mindControlledEnemyBT = mPickedTarget->GetComponent<Script_MindControlledEnemyBT>();
	mindControlledEnemyBT->SetActive(true);
	mindControlledEnemyBT->SetInvoker(mObject);
}

void Script_Ability_MindControl::ActivePrevEnemyBT()
{
	sptr<Script_MindControlledEnemyBT> mindControlledEnemyBT;
	if (mindControlledEnemyBT = mPickedTarget->GetComponent<Script_MindControlledEnemyBT>()) {
		mindControlledEnemyBT->SetActive(false);
	}

	for (const auto& component : mPickedTarget->GetAllComponents()) {
		if (component == mindControlledEnemyBT) {
			continue;
		}

		if (std::dynamic_pointer_cast<Script_BehaviorTree>(component)) {
			component->SetActive(true);
			break;
		}
	}
}

void Script_Ability_MindControl::Click()
{
	if (mCurrControlledObjectCnt > 0) {
		return;
	}

	mPickedTarget = PickingObject(InputMgr::I->GetMousePos());

	if (mPickedTarget) {
		if (!ReducePheroAmount()) {
			SetActive(false);
			return;
		}

		ActiveMindControlledEnemyBT();
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
