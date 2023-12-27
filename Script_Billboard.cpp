#include "stdafx.h"
#include "Script_Billboard.h"
#include "Scene.h"
#include "Camera.h"

void Script_Billboard::Start()
{
	mCameraObject = crntScene->GetCameraObject();
}

void Script_Billboard::Update()
{
	mObject->LookAt(mCameraObject->GetPosition(), Vector3::Up());
}
