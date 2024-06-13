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

		// TODO : 에임 마우스 좌표가 -1920 ~ 1920까지인 문제
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
	// TODO : 추후에 또 사용될 수 있으므로 Picking을 엔진 코드로 옮겨야 한다.
	if (!mCamera) {
		return nullptr;
	}

	Matrix mtxProj = mCamera->GetProjMtx();

	// 스크린 좌표계를 투영 좌표계로
	float viewX = (2.f * screenPos.x / mWindowWidth - 1.f) / mtxProj(0, 0);
	float viewY = (-2.f * screenPos.y / mWindowHeight + 1.f) / mtxProj(1, 1);

	Matrix mtxView = mCamera->GetViewMtx();
	Matrix mtxViewInv = mtxView.Invert();

	float minDistance = FLT_MAX;
	Object* pickedObject = nullptr;
	
	// 주변 그리드 내 모든 적 객체
	const std::vector<sptr<Grid>>& grids = Scene::I->GetNeighborGrids(Scene::I->GetGridIndexFromPos(mObject->GetPosition()), true);
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
