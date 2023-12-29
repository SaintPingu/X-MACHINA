#include "stdafx.h"
#include "Camera.h"
#include "DXGIMgr.h"

#include "Collider.h"
#include "Object.h"
#include "Timer.h"
#include "Scene.h"

#include "Script_MainCamera.h"

SINGLETON_PATTERN_DEFINITION(MainCameraObject)





// [ Camera ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

void Camera::Start()
{
	CreateShaderVariables();
}

void Camera::Release()
{
	ReleaseShaderVariables();
}

//////////////////* Shader Variables *//////////////////
void Camera::CreateShaderVariables()
{
	UINT cbByteSize = D3DUtil::CalcConstantBuffSize(sizeof(*mCBMap_Camera));
	D3DUtil::CreateBufferResource(nullptr, cbByteSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, mCBCamera);
	mCBCamera->Map(0, nullptr, (void**)&mCBMap_Camera);
}


void Camera::UpdateShaderVariables()
{
	Vec4x4 viewMtx = Matrix4x4::Transpose(mViewTransform);
	Vec4x4 projMtx = Matrix4x4::Transpose(mProjectionTransform);

	::memcpy(&mCBMap_Camera->mView, &viewMtx, sizeof(Vec4x4));

	::memcpy(&mCBMap_Camera->mProjection, &projMtx, sizeof(Vec4x4));

	Vec3 pos = mObject->GetPosition();
	::memcpy(&mCBMap_Camera->mPosition, &pos, sizeof(Vec3));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = mCBCamera->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(scene->GetRootParamIndex(RootParam::Camera), gpuVirtualAddress);
}


void Camera::ReleaseShaderVariables()
{
	if (mCBCamera) {
		mCBCamera->Unmap(0, nullptr);
		mCBCamera = nullptr;
	}
}



//////////////////* View Matrix *//////////////////a
void Camera::GenerateViewMatrix()
{
	mViewTransform = Matrix4x4::LookAtLH(mObject->GetPosition(), mObject->GetLook(), mObject->GetUp());
}


void Camera::RegenerateViewMatrix()
{
	Vec3 pos = mObject->GetPosition();
	Vec3 right = mObject->GetRight();
	Vec3 up = mObject->GetUp();
	Vec3 look = mObject->GetLook();

	mViewTransform._11 = right.x; mViewTransform._12 = up.x; mViewTransform._13 = look.x;
	mViewTransform._21 = right.y; mViewTransform._22 = up.y; mViewTransform._23 = look.y;
	mViewTransform._31 = right.z; mViewTransform._32 = up.z; mViewTransform._33 = look.z;
	mViewTransform._41 = -Vector3::DotProduct(pos, right);
	mViewTransform._42 = -Vector3::DotProduct(pos, up);
	mViewTransform._43 = -Vector3::DotProduct(pos, look);

	CalculateFrustumPlanes();
}


void Camera::GenerateProjectionMatrix(float nearPlaneDistance, float farPlaneDistance, float aspectRatio, float fovAngle)
{
	Matrix projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngle), aspectRatio, nearPlaneDistance, farPlaneDistance);
	XMStoreFloat4x4(&mProjectionTransform, projection);

	BoundingFrustum::CreateFromMatrix(mFrustumView, projection);
}



//////////////////* Viewport *//////////////////
void Camera::SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ, float maxZ)
{
	mViewport.TopLeftX = float(xTopLeft);
	mViewport.TopLeftY = float(yTopLeft);
	mViewport.Width = float(width);
	mViewport.Height = float(height);
	mViewport.MinDepth = minZ;
	mViewport.MaxDepth = maxZ;
}


void Camera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	mScissorRect.left = xLeft;
	mScissorRect.top = yTop;
	mScissorRect.right = xRight;
	mScissorRect.bottom = yBottom;
}


void Camera::SetViewportsAndScissorRects()
{
	cmdList->RSSetViewports(1, &mViewport);
	cmdList->RSSetScissorRects(1, &mScissorRect);
}



//////////////////* Transform *//////////////////


void Camera::LookAt(const Vec3& lookAt, const Vec3& upVector)
{
	Vec4x4 mtxLookAt = Matrix4x4::LookAtLH(mObject->GetPosition(), lookAt, upVector);
	Vec3 right = Vec3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	Vec3 up = Vec3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	Vec3 look = Vec3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
	mObject->SetAxis(look, up, right);
}



//////////////////* Frustum *//////////////////
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



///////////////////////////* Others *///////////////////////////



CameraObject::CameraObject() : Object()
{
	mCamera = AddComponent<Camera>();
}



void CameraObject::Rotate(float pitch, float yaw, float roll)
{
	Transform::Rotate(pitch, yaw, roll);
	SetRightY(0.0f);
}


void CameraObject::LookAt(const Vec3& lookAt, const Vec3& up)
{
	mCamera->LookAt(lookAt, up);
}








Vec3 MainCameraObject::GetPlayerPos() const
{
	return mPlayer->GetPosition();
}


void MainCameraObject::Start()
{
	SetPlayer(scene->GetPlayer());
	SetPosition(0, 0, -1);	// must be non-zero
	AddComponent<Script_MainCamera>();

	CameraObject::Start();
}

void MainCameraObject::LookAt(const Vec3& lookAt)
{
	CameraObject::LookAt(lookAt, mPlayer->GetUp());
}


void MainCameraObject::LookPlayer()
{
	if (mPlayer) {
		LookAt(mPlayer->GetPosition());
	}
}