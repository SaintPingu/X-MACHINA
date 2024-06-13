#include "stdafx.h"
#include "MindControlAbility.h"

#include "Object.h"
#include "GameFramework.h"
#include "Grid.h"
#include "Scene.h"
#include "InputMgr.h"

#include "Component/Camera.h"
#include "Component/Collider.h"
#include "Script_Player.h"
#include "Script_MainCamera.h"
#include "Script_AimController.h"


MindControlAbility::MindControlAbility()
	:
	RenderedAbility(10.f, 30.f),
	PheroAbilityInterface(0.f),
	mWindowWidth(static_cast<float>(GameFramework::I->GetWindowResolution().Width)),
	mWindowHeight(static_cast<float>(GameFramework::I->GetWindowResolution().Height))
{
	mCamera = MainCamera::I->GetCamera();
}

void MindControlAbility::Update(float activeTime)
{
	base::Update(activeTime);

	if (KEY_TAP(VK_LBUTTON)) {
		sptr<Script_AimController> aim = mObject->GetComponent<Script_AimController>();
		if (!aim) {
			return;
		}

		// TODO : ���� ���콺 ��ǥ�� -1920 ~ 1920������ ����
		Vec2 screenPos = aim->GetAimPos() / 2.f;
		screenPos.y = 1 - screenPos.y;
		
		screenPos += Vec2{ mWindowWidth, mWindowHeight } / 2.f;

		Object* mPickedObject = PickingObject(screenPos);

		if (mPickedObject) {
			mPickedObject->GetComponent<Script_LiveObject>()->Dead();
		}
	}
}

bool MindControlAbility::Activate()
{
	if (!ReducePheroAmount()) {
		return false;
	}

	if (!RenderedAbility::Activate()) {
		return false;
	}

	return true;
}

void MindControlAbility::DeActivate()
{
	base::DeActivate();
}

bool MindControlAbility::ReducePheroAmount()
{
	sptr<Script_PheroPlayer> pheroPlayer = mObject->GetComponent<Script_PheroPlayer>();
	if (pheroPlayer) {
		return pheroPlayer->ReducePheroAmount(mPheroCost);
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
	const std::vector<sptr<Grid>>& grids = Scene::I->GetNeighborGrids(Scene::I->GetGridIndexFromPos(mObject->GetPosition()), true);
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
