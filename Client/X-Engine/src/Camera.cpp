#include "EnginePch.h"
#include "Component/Camera.h"
#include "DXGIMgr.h"

#include "Object.h"
#include "Component/Collider.h"
#include "BattleScene.h"





#pragma region Camera
void Camera::Awake()
{
	base::Awake();

	float width = DXGIMgr::I->GetWindowWidth();
	float height = DXGIMgr::I->GetWindowHeight();

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

void Camera::SetLens(float fovY, float aspect, float farPlaneDistance, float fovAngle)
{
	const XMMATRIX kProjMtx = XMMatrixPerspectiveFovLH(fovY, aspect, farPlaneDistance, fovAngle);
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
	CMD_LIST->RSSetViewports(1, &mViewport);
	CMD_LIST->RSSetScissorRects(1, &mScissorRect);
}


void Camera::LookAt(const Vec3& lookAt, const Vec3& upVector)
{
	mObject->SetAxis(Matrix4x4::LookAtLH(mObject->GetPosition(), lookAt, upVector, true));
}

Vec2 Camera::WorldToScreenPoint(const Vec3& pos)
{
	// "pos" to "clip coord"
	XMVECTOR screenPoint = XMVector3TransformCoord(XMVector3TransformCoord(_VECTOR(pos), _MATRIX(mViewTransform)), _MATRIX(mProjTransform));

	// "clip coord" to "NDC"
	screenPoint /= XMVectorGetW(screenPoint);

	// "NDC" to "screen coord"
	const float halfWidth = mViewport.Width * 0.5f;
	const float halfHeight = mViewport.Height * 0.5f;
	screenPoint = XMVectorMultiplyAdd(screenPoint, XMVectorSet(halfWidth, halfHeight, 0.0f, 0.0f), XMVectorSet(halfWidth, halfHeight, 0.0f, 0.0f));
	screenPoint = XMVectorSubtract(screenPoint, XMVectorSet(halfWidth, halfHeight, 0.f, 0.f));

	Vec3 result;
	XMStoreFloat3(&result, screenPoint);
	return Vec2(result.x, result.y);
}

Vec3 Camera::ScreenToWorldRay(const Vec2& pos)
{
	// screen coord -> NDC(Normalized Device Coordinates) -> clip coord
	const Vec2 ndc = ScreenToNDC(pos);

	Vec3 pickPos;
	pickPos.x = ndc.x / mProjTransform._11;
	pickPos.y = ndc.y / mProjTransform._22;
	pickPos.z = 1.f;

	// Calculate the difference with the camera after converting to world space //
	const Matrix inverse = Matrix4x4::Inverse(mViewTransform);
	const Vec3 world = Vec3::Transform(pickPos, inverse);		// Front of camera

 	return Vector3::Normalized(world - mObject->GetPosition());
}

Vec2 Camera::ScreenToNDC(const Vec2& pos)
{
	return Vec2(pos.x / (mViewport.Width * 0.5f), pos.y / (mViewport.Height * 0.5f));
}

void Camera::CalculateFrustumPlanes()
{
	mFrustumView.Transform(mFrustumWorld, XMMatrixInverse(nullptr, _MATRIX(mViewTransform)));
	
	// shadow camera frustum //
	std::memcpy(&mFrustumWorldShadow, &mFrustumWorld, sizeof(mFrustumWorld));
	mFrustumWorldShadow.Origin.y += mOffset.y * 2;
	mFrustumWorldShadow.Origin.z += mOffset.z * 2;
}
#pragma endregion





#pragma region CameraObject
CameraObject::CameraObject() : Object()
{
	mCamera = AddComponent<Camera>();
}

void CameraObject::LookAt(const Vec3& lookAt, const Vec3& up)
{
	mCamera->LookAt(lookAt, up);
}
#pragma endregion





#pragma region MainCamera
void MainCamera::Awake()
{
	base::Awake();

	SetPosition(1, 2, 3);	// must be non-zero
}
#pragma endregion