#include "stdafx.h"
#include "Camera.h"
#include "DXGIMgr.h"

#include "Collider.h"
#include "Object.h"
#include "Timer.h"
#include "Scene.h"

#include "Script_MainCamera.h"







// [ Camera ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//===== (Camera) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////


//===== (Camera) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////

Vec3 Camera::GetPosition()
{
	return mLocalTransform->GetPosition();
}

//===== (Camera) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////


//===== (Camera) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

//////////////////* Shader Variables *//////////////////
void Camera::CreateShaderVariables()
{
	UINT cbBytes = ((sizeof(*mCbMappedCamera) + 255) & ~255);
	::CreateBufferResource(nullptr, cbBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, mCbCamera);
	mCbCamera->Map(0, nullptr, (void**)&mCbMappedCamera);
}


void Camera::UpdateShaderVariables()
{
	::memcpy(&mCbMappedCamera->mView, &Matrix4x4::Transpose(mViewTransform), sizeof(Vec4x4));

	::memcpy(&mCbMappedCamera->mProjection, &Matrix4x4::Transpose(mProjectionTransform), sizeof(Vec4x4));

	Vec3 pos = mLocalTransform->GetPosition();
	::memcpy(&mCbMappedCamera->mPosition, &pos, sizeof(Vec3));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = mCbCamera->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(crntScene->GetRootParamIndex(RootParam::Camera), gpuVirtualAddress);
}


void Camera::ReleaseShaderVariables()
{
	if (mCbCamera) {
		mCbCamera->Unmap(0, nullptr);
	}
}



//////////////////* View Matrix *//////////////////a
void Camera::GenerateViewMatrix()
{
	mViewTransform = Matrix4x4::LookAtLH(mLocalTransform->GetPosition(), mLookAtWorld, mLocalTransform->GetUp());
}


void Camera::RegenerateViewMatrix()
{
	Vec3 pos = mLocalTransform->GetPosition();
	Vec3 right = mLocalTransform->GetRight();
	Vec3 up = mLocalTransform->GetUp();
	Vec3 look = mLocalTransform->GetLook();

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
	Vec4x4 mtxLookAt = Matrix4x4::LookAtLH(mLocalTransform->GetPosition(), lookAt, upVector);
	Vec3 right = Vec3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	Vec3 up = Vec3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	Vec3 look = Vec3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
	mLocalTransform->SetAxis(look, up, right);
}



//////////////////* Frustum *//////////////////
void Camera::CalculateFrustumPlanes()
{
	mFrustumView.Transform(mFrustumWorld, XMMatrixInverse(nullptr, XMLoadFloat4x4(&mViewTransform)));
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
void Camera::Start()
{
	mLocalTransform = mObject;
}


void Camera::Update()
{
}





CameraObject::CameraObject() : Object()
{
	mCamera = AddComponent<Camera>();
}


void CameraObject::CreateShaderVariables()
{
	mCamera->CreateShaderVariables();
}


void CameraObject::ReleaseShaderVariables()
{
	mCamera->ReleaseShaderVariables();
}


void CameraObject::UpdateShaderVariables()
{
	mCamera->UpdateShaderVariables();
}




void CameraObject::Rotate(float pitch, float yaw, float roll)
{
	Transform::Rotate(pitch, yaw, roll);
	SetRightY(0.0f);
}


void CameraObject::MoveLocal(const Vec3& xmf3Shift)
{
	Transform::Translate(xmf3Shift);
}


void CameraObject::LookAt(const Vec3& lookAt, const Vec3& up)
{
	mCamera->LookAt(lookAt, up);
}


void CameraObject::Start()
{
	Object::Start();
}








Vec3 MainCamera::GetPlayerPos() const
{
	return mPlayer->GetPosition();
}


void MainCamera::Start()
{
	SetPlayer(crntScene->GetPlayer());
	SetPosition(0, 0, -1);	// must be non-zero
	AddComponent<Script_MainCamera>();
	CameraObject::Start();
}


void MainCamera::Update()
{
	CameraObject::Update();
}

void MainCamera::LookAt(const Vec3& lookAt)
{
	CameraObject::LookAt(lookAt, mPlayer->GetUp());
}


void MainCamera::LookPlayer()
{
	if (mPlayer) {
		LookAt(mPlayer->GetPosition());
	}
}