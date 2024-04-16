#include "EnginePch.h"
#include "Component/Camera.h"
#include "DXGIMgr.h"

#include "Object.h"
#include "Component/Collider.h"
#include "Scene.h"





#pragma region Camera
void Camera::Awake()
{
	base::Awake();

	float width = dxgi->GetWindowWidth();
	float height = dxgi->GetWindowHeight();

	mAspectRatio = width / height;

	mViewport    = { 0.f, 0.f, width, height, 0.f, 1.f };
	mScissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height)};
}

void Camera::UpdateViewMtx()
{
	const Vec3 pos   = mObject->GetPosition();
	const Vec3 right = mObject->GetRight();
	const Vec3 up    = mObject->GetUp();
	const Vec3 look  = mObject->GetLook();

	mViewTransform._11 = right.x; mViewTransform._12 = up.x; mViewTransform._13 = look.x;
	mViewTransform._21 = right.y; mViewTransform._22 = up.y; mViewTransform._23 = look.y;
	mViewTransform._31 = right.z; mViewTransform._32 = up.z; mViewTransform._33 = look.z;
	mViewTransform._41 = -pos.Dot(right);
	mViewTransform._42 = -pos.Dot(up);
	mViewTransform._43 = -pos.Dot(look);

	CalculateFrustumPlanes();
}
void Camera::SetProjMtx(float nearPlaneDistance, float farPlaneDistance, float fovAngle)
{
	const Matrix kProjMtx = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngle), mAspectRatio, nearPlaneDistance, farPlaneDistance);
	XMStoreFloat4x4(&mProjTransform, kProjMtx);

	BoundingFrustum::CreateFromMatrix(mFrustumView, kProjMtx);
}


void Camera::SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ, float maxZ)
{
	mViewport.TopLeftX = float(xTopLeft);
	mViewport.TopLeftY = float(yTopLeft);
	mViewport.Width    = float(width);
	mViewport.Height   = float(height);
	mViewport.MinDepth = minZ;
	mViewport.MaxDepth = maxZ;
}
void Camera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	mScissorRect.left   = xLeft;
	mScissorRect.top    = yTop;
	mScissorRect.right  = xRight;
	mScissorRect.bottom = yBottom;
}
void Camera::SetViewportsAndScissorRects()
{
	cmdList->RSSetViewports(1, &mViewport);
	cmdList->RSSetScissorRects(1, &mScissorRect);
}


void Camera::LookAt(const Vec3& lookAt, const Vec3& upVector)
{
	mObject->SetAxis(Matrix4x4::LookAtLH(mObject->GetPosition(), lookAt, upVector, true));
}

Vec2 Camera::WorldToScreenPoint(const Vec3& pos)
{
	// position을 클립 좌표 공간으로 변환
	XMVECTOR screenPoint = XMVector3TransformCoord(XMVector3TransformCoord(_VECTOR(pos), _MATRIX(mViewTransform)), _MATRIX(mProjTransform));

	// 클립 좌표를 NDC로 변환
	screenPoint /= XMVectorGetW(screenPoint);

	// NDC를 screen 좌표로 변환
	screenPoint = XMVectorMultiplyAdd(screenPoint, XMVectorSet(mViewport.Width, mViewport.Height, 0.0f, 0.0f), XMVectorSet(mViewport.Width * 0.5f, mViewport.Height * 0.5f, 0.0f, 0.0f));
	screenPoint = XMVectorSubtract(screenPoint, XMVectorSet(mViewport.Width * 0.5f, mViewport.Height * 0.5f, 0.f, 0.f));

	Vec3 result;
	XMStoreFloat3(&result, screenPoint);
	return Vec2(result.x, result.y);
}

Vec3 Camera::ScreenToWorldRay(const Vec2& pos)
{
	// 스크린 좌표 -> NDC(Normalized Device Coordinates) -> 클립 좌표로 변환
	const Vec2 ndc = ScreenToNDC(pos);

	Vec3 pickPos;
	pickPos.x = ndc.x / mProjTransform._11;
	pickPos.y = ndc.y / mProjTransform._22;
	pickPos.z = 1.f;

	// 월드공간으로 변환 후 카메라와의 차이 계산
	const Matrix inverse = Matrix4x4::Inverse(mViewTransform);
	const Vec3 world = Vec3::Transform(pickPos, inverse); // front of camera

 	return Vector3::Normalized(world - mObject->GetPosition());
}

Vec2 Camera::ScreenToNDC(const Vec2& pos)
{
	return Vec2(pos.x / mViewport.Width, pos.y / mViewport.Height);
}

void Camera::CalculateFrustumPlanes()
{
	mFrustumView.Transform(mFrustumWorld, XMMatrixInverse(nullptr, _MATRIX(mViewTransform)));
}
#pragma endregion





#pragma region CameraObject
CameraObject::CameraObject() : Object()
{
	mCamera = AddComponent<Camera>();
}


void CameraObject::Rotate(float pitch, float yaw, float roll)
{
	base::Rotate(pitch, yaw, roll);

	SetRightY(0.f);	// Roll 회전 x
}


void CameraObject::LookAt(const Vec3& lookAt, const Vec3& up)
{
	mCamera->LookAt(lookAt, up);
}
#pragma endregion





#pragma region MainCameraObject
void MainCameraObject::Awake()
{
	base::Awake();

	SetPosition(1, 2, 3);	// must be non-zero
}
#pragma endregion