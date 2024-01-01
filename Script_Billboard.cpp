#include "stdafx.h"
#include "Script_Billboard.h"

#include "Scene.h"
#include "Camera.h"


void Script_Billboard::Update()
{
	mObject->LookAt(mainCameraObject->GetPosition(), Vector3::Up());
}

void Script_Billboard::UpdateSpriteVariable() const
{
	if (!Math::IsEqual(mScale, 1.f)) {
		Matrix mtxScale = XMMatrixMultiply(XMMatrixScaling(mScale, mScale, 1.f), _MATRIX(mObject->GetWorldTransform()));
		scene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(mtxScale), 0);
	}
}