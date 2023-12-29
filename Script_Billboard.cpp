#include "stdafx.h"
#include "Script_Billboard.h"
#include "Scene.h"
#include "Camera.h"

void Script_Billboard::Start()
{
	mCameraObject = mainCameraObject;
}

void Script_Billboard::Update()
{
	mObject->LookAt(mCameraObject->GetPosition(), Vector3::Up());
}
