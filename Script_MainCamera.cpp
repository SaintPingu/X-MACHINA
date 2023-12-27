#include "stdafx.h"
#include "Script_MainCamera.h"
#include "Object.h"
#include "Timer.h"
#include "Camera.h"
#include "Scene.h"


void Script_MainCamera::Start()
{
	mCameraObject = mObject->GetObj<MainCamera>();
	mPlayer = mCameraObject->GetPlayer();
	mCamera = mCameraObject->GetCamera().get();

	ChangeCameraMode(CameraMode::Third);
}


void Script_MainCamera::Update()
{
	Vec4x4 rotationMtx = Matrix4x4::Identity();
	Vec3 right = mPlayer->GetRight();
	Vec3 up = mPlayer->GetUp();
	Vec3 look = mPlayer->GetLook();

	rotationMtx._11 = right.x; rotationMtx._21 = up.x; rotationMtx._31 = look.x;
	rotationMtx._12 = right.y; rotationMtx._22 = up.y; rotationMtx._32 = look.y;
	rotationMtx._13 = right.z; rotationMtx._23 = up.z; rotationMtx._33 = look.z;

	Vec3 offset = Vector3::TransformCoord(mOffset, rotationMtx);
	Vec3 position = Vector3::Add(mPlayer->GetPosition(), offset);
	Vec3 dir = Vector3::Subtract(position, mObject->GetPosition());

	float length = Vector3::Length(dir);
	dir = Vector3::Normalize(dir);

	float timeScale = (mTimeLag) ? DeltaTime() * (1.0f / mTimeLag) : 1.0f;
	float distance = length * timeScale;

	if (distance > length) {
		distance = length;
	}
	if (length < 0.01f) {
		distance = length;
	}
	if (distance > 0)
	{
		mCameraObject->Translate(dir, distance);
	}

	mCameraObject->LookPlayer();
	mCamera->RegenerateViewMatrix();
}

void Script_MainCamera::UpdateHeight()
{
	constexpr float minHeight = 2.0f;

	if (mCameraMode == CameraMode::Third) {
		if (mObject->GetPosition().y <= minHeight) {
			mObject->SetPositionY(minHeight);
			mCameraObject->LookPlayer();
		}
	}

	Vec3 camPos = mObject->GetPosition();

	float height = crntScene->GetTerrainHeight(camPos.x, camPos.z) + minHeight;
	if (camPos.y <= height)
	{
		camPos.y = height;
		mObject->SetPosition(camPos);
	}
}



void Script_MainCamera::SetCameraOffset(const Vec3& offset)
{
	Vec3 playerPos = mCameraObject->GetPlayerPos();
	mOffset = offset;
	mCamera->SetOffset(mOffset);
	mCamera->LookAt(playerPos, mPlayer->GetUp());
	mCamera->GenerateViewMatrix();
}

void Script_MainCamera::RotateOffset(const Vec3& axis, float angle)
{
	Vec3 cameraOffset = mCamera->GetOffset();
	XMMATRIX mtxRotation = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(angle));
	XMVECTOR xmNewOffset = XMVector3TransformCoord(XMLoadFloat3(&cameraOffset), mtxRotation);
	XMStoreFloat3(&cameraOffset, xmNewOffset);
	mCamera->SetOffset(cameraOffset);
}

void Script_MainCamera::ChangeCameraMode(CameraMode mode)
{
	constexpr float maxPlaneDistance = 2500.0f;

	if (mCameraMode == mode) {
		return;
	}

	switch (mode) {
	case CameraMode::Third:
		mTimeLag = 0.1f;
		SetCameraOffset(Vec3(0.0f, 15, -30.0f));
		mCamera->GenerateProjectionMatrix(1.01f, maxPlaneDistance, ASPECT_RATIO, 80.0f);
		mCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		mCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		assert(0);
		break;
	}

	mCameraMode = mode;
}