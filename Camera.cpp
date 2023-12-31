#include "stdafx.h"
#include "Camera.h"
#include "DXGIMgr.h"

#include "Object.h"
#include "Collider.h"
#include "Scene.h"

#include "Script_MainCamera.h"





#pragma region Camera
void Camera::Start()
{
	base::Start();

	CreateShaderVars();
}
void Camera::Release()
{
	base::Release();

	ReleaseShaderVars();
}


void Camera::UpdateShaderVars()
{
	Vec4x4 viewMtx = Matrix4x4::Transpose(mViewTransform);
	Vec4x4 projMtx = Matrix4x4::Transpose(mProjectionTransform);

	::memcpy(&mCBMap_Camera->View, &viewMtx, sizeof(Vec4x4));

	::memcpy(&mCBMap_Camera->Projection, &projMtx, sizeof(Vec4x4));

	Vec3 pos = mObject->GetPosition();
	::memcpy(&mCBMap_Camera->Position, &pos, sizeof(Vec3));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = mCB_Camera->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(scene->GetRootParamIndex(RootParam::Camera), gpuVirtualAddress);
}


void Camera::UpdateViewMtx()
{
	Vec3 pos   = mObject->GetPosition();
	Vec3 right = mObject->GetRight();
	Vec3 up    = mObject->GetUp();
	Vec3 look  = mObject->GetLook();

	mViewTransform._11 = right.x; mViewTransform._12 = up.x; mViewTransform._13 = look.x;
	mViewTransform._21 = right.y; mViewTransform._22 = up.y; mViewTransform._23 = look.y;
	mViewTransform._31 = right.z; mViewTransform._32 = up.z; mViewTransform._33 = look.z;
	mViewTransform._41 = -Vector3::DotProduct(pos, right);
	mViewTransform._42 = -Vector3::DotProduct(pos, up);
	mViewTransform._43 = -Vector3::DotProduct(pos, look);

	CalculateFrustumPlanes();
}
void Camera::SetProjMtx(float nearPlaneDistance, float farPlaneDistance, float aspectRatio, float fovAngle)
{
	Matrix projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngle), aspectRatio, nearPlaneDistance, farPlaneDistance);
	XMStoreFloat4x4(&mProjectionTransform, projection);

	BoundingFrustum::CreateFromMatrix(mFrustumView, projection);
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
	Vec4x4 mtxLookAt = Matrix4x4::LookAtLH(mObject->GetPosition(), lookAt, upVector);
	Vec3 right       = Vec3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	Vec3 up          = Vec3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	Vec3 look        = Vec3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);

	mObject->SetAxis(look, up, right);
}


void Camera::CalculateFrustumPlanes()
{
	mFrustumView.Transform(mFrustumWorld, XMMatrixInverse(nullptr, _MATRIX(mViewTransform)));
}
bool Camera::IsInFrustum(const BoundingOrientedBox& boundingBox)
{
	return mFrustumWorld.Intersects(boundingBox);
}
bool Camera::IsInFrustum(const BoundingBox& boundingBox)
{
	return mFrustumWorld.Intersects(boundingBox);
}
bool Camera::IsInFrustum(const BoundingSphere& boundingSphere)
{
	return mFrustumWorld.Intersects(boundingSphere);
}
bool Camera::IsInFrustum(rsptr<const GameObject> object)
{
	return object->GetComponent<ObjectCollider>()->IsInFrustum(mFrustumWorld);
}


void Camera::CreateShaderVars()
{
	const UINT cbByteSize = D3DUtil::CalcConstantBuffSize(sizeof(*mCBMap_Camera));
	D3DUtil::CreateBufferResource(nullptr, cbByteSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, mCB_Camera);
	mCB_Camera->Map(0, nullptr, (void**)&mCBMap_Camera);
}
void Camera::ReleaseShaderVars()
{
	if (mCB_Camera) {
		mCB_Camera->Unmap(0, nullptr);
		mCB_Camera = nullptr;
	}
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
	SetRightY(0.f);
}


void CameraObject::LookAt(const Vec3& lookAt, const Vec3& up)
{
	mCamera->LookAt(lookAt, up);
}
#pragma endregion





#pragma region MainCameraObject
SINGLETON_PATTERN_DEFINITION(MainCameraObject)

void MainCameraObject::Start()
{
	SetPosition(0, 0, -1);	// must be non-zero
	AddComponent<Script_MainCamera>();

	base::Start();
}
#pragma endregion