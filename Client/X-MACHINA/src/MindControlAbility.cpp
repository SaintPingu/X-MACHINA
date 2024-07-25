#include "stdafx.h"
#include "MindControlAbility.h"

#include "Object.h"
#include "GameFramework.h"
#include "Grid.h"
#include "BattleScene.h"
#include "InputMgr.h"
#include "ResourceMgr.h"
#include "Texture.h"

#include "Component/Camera.h"
#include "Component/Collider.h"
#include "Component/UI.h"

#include "Script_Player.h"
#include "Script_MainCamera.h"
#include "Script_AimController.h"
#include "Script_DefaultEnemyBT.h"
#include "Script_MindControlledEnemyBT.h"
#include "Script_BehaviorTree.h"


MindControlAbility::MindControlAbility()
	:
	RenderedAbility("MindControl", 10.f, 30.f),
	PheroAbilityInterface(200.f),
	mWindowWidth(static_cast<float>(GameFramework::I->GetWindowResolution().Width)),
	mWindowHeight(static_cast<float>(GameFramework::I->GetWindowResolution().Height))
{
	mCamera = MainCamera::I->GetCamera();
	mMaxControlledObjectCnt = 1;
	mCurrControlledObjectCnt = mMaxControlledObjectCnt;
	mMindControlAimTexture = RESOURCE<Texture>("MindControlAim");
}

void MindControlAbility::Update(float activeTime)
{
	base::Update(activeTime);

	if (KEY_TAP(VK_LBUTTON) && mCurrControlledObjectCnt > 0) {

		mPickedTarget = PickingObject(InputMgr::I->GetMousePos());

		if (mPickedTarget) {
			if (!ReducePheroAmount()) {
				Terminate();
				return;
			}

			ActiveMindControlledEnemyBT();
			mCurrControlledObjectCnt--;
		}

		if (mCurrControlledObjectCnt <= 0) {
			ChangeAimToOrigin();
		}
	}
	else if (KEY_TAP(mHolderKey) && mCurrControlledObjectCnt > 0) {
		Terminate();
	}
}

void MindControlAbility::Activate()
{
	if (!ReducePheroAmount(true)) {
		mTerminateCallback();
		return;
	}

	RenderedAbility::Activate();

	mCurrControlledObjectCnt = mMaxControlledObjectCnt;

	mAimController = mObject->GetComponent<Script_AimController>();
	if (!mAimController) {
		Terminate();
		return;
	}

	ChangeAimToActive();
}

void MindControlAbility::DeActivate()
{
	base::DeActivate();

	if (mPickedTarget) {
		ActivePrevEnemyBT();
	}
}

bool MindControlAbility::ReducePheroAmount(bool checkOnly)
{
	sptr<Script_PheroPlayer> pheroPlayer = mObject->GetComponent<Script_PheroPlayer>();
	if (pheroPlayer) {
		return pheroPlayer->ReducePheroAmount(mPheroCost, checkOnly);
	}

	return false;
}

Object* MindControlAbility::PickingObject(const Vec2& screenPos)
{
	// TODO : ���Ŀ� �� ���� �� �����Ƿ� Picking�� ���� �ڵ�� �Űܾ� �Ѵ�.
	if (!mCamera) {
		return nullptr;
	}

	Matrix mtxProj = mCamera->GetProjMtx();

	// ��ũ�� ��ǥ�踦 ���� ��ǥ���
	float viewX = (2.f * screenPos.x / mWindowWidth - 1.f) / mtxProj(0, 0);
	float viewY = (-2.f * screenPos.y / mWindowHeight + 1.f) / mtxProj(1, 1);

	Matrix mtxView = mCamera->GetViewMtx();
	Matrix mtxViewInv = mtxView.Invert();

	float minDistance = FLT_MAX;
	Object* pickedObject = nullptr;

	// �ֺ� �׸��� �� ��� �� ��ü
	const std::vector<sptr<Grid>>& grids = BattleScene::I->GetNeighborGrids(BattleScene::I->GetGridIndexFromPos(mObject->GetPosition()), true);
	for (const auto& grid : grids) {
		for (const auto& object : grid->GetObjectsFromTag(ObjectTag::Enemy)) {
			Ray ray(Vec3{ 0.f, 0.f, 0.f }, Vec3{ viewX, viewY, 1.f });

			// �� ��ǥ�踦 ���� ��ǥ���
			ray.Position = XMVector3TransformCoord(ray.Position, mtxViewInv);
			ray.Direction = XMVector3TransformNormal(ray.Direction, mtxViewInv);
			ray.Direction.Normalize();

			// ���� ��ǥ�迡�� ���̿� �ٿ�� ���Ǿ��� �浹 �˻�
			float distance = 0.f;
			if (ray.Intersects(object->GetCollider()->GetBS(), distance) == false)
				continue;

			// �Ÿ��� ���� ª�� ��ü ��ŷ ����
			if (distance < minDistance) {
				minDistance = distance;
				pickedObject = object;
			}
		}
	}

	return pickedObject;
}

void MindControlAbility::ActiveMindControlledEnemyBT()
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

void MindControlAbility::ActivePrevEnemyBT()
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

void MindControlAbility::Terminate()
{
	ChangeAimToOrigin();
	mTerminateCallback();
}

void MindControlAbility::ChangeAimToOrigin()
{
	mAimController->ChangeAimTexture(mPrevAimTexture, mPrevAimScale);
}

void MindControlAbility::ChangeAimToActive()
{
	mPrevAimTexture = mAimController->GetTexture();
	mPrevAimScale = mAimController->GetTextureScale();
	mAimController->ChangeAimTexture(mMindControlAimTexture, Vec2(300.f, 300.f));
}
