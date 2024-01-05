#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region Define
#define mainCameraObject MainCameraObject::Inst()
#define mainCamera mainCameraObject->GetCamera()
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
#pragma endregion


#pragma region Variable
constexpr float gkAspectRatio = (float(gkFrameBufferWidth) / float(gkFrameBufferHeight));
#pragma endregion


#pragma region Struct
struct CB_CameraInfo {
	Vec4x4 View{};			// View 행렬
	Vec4x4 Projection{};	// Projection 행렬
	Vec3   Position{};		// 카메라 위치
};
#pragma endregion


#pragma region Class

class Camera : public Component {
	COMPONENT(Camera, Component)

private:
	Vec4x4 mViewTransform = Matrix4x4::Identity();
	Vec4x4 mProjTransform = Matrix4x4::Identity();

	// Camera Constant Buffer
	ComPtr<ID3D12Resource>	mCB_CameraInfo{};
	CB_CameraInfo*			mCBMap_CameraInfo{};

	Vec3 mOffset{};

	BoundingFrustum mFrustumView{};
	BoundingFrustum mFrustumWorld{};

	D3D12_VIEWPORT	mViewport    = { 0.f, 0.f, gkFrameBufferWidth , gkFrameBufferHeight, 0.f, 1.f };
	D3D12_RECT		mScissorRect = { 0, 0, gkFrameBufferWidth , gkFrameBufferHeight };

public:
	Vec3 GetOffset() const { return mOffset; }

	void SetOffset(const Vec3& offset) { mOffset = offset; }

public:
	virtual void Start() override;
	virtual void Release() override;

public:
	void UpdateShaderVars();

	void UpdateViewMtx();
	void SetProjMtx(float nearPlaneDistance, float farPlaneDistance, float aspectRatio, float fovAngle);
		 
	void SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ = 0.f, float maxZ = 1.f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);
	void SetViewportsAndScissorRects();

	void LookAt(const Vec3& lookAt, const Vec3& up);

	bool IsInFrustum(const BoundingBox& boundingBox)		 { return mFrustumWorld.Intersects(boundingBox); }
	bool IsInFrustum(const BoundingOrientedBox& boundingBox) { return mFrustumWorld.Intersects(boundingBox); }
	bool IsInFrustum(const BoundingSphere& boundingSphere)   { return mFrustumWorld.Intersects(boundingSphere); }
	bool IsInFrustum(rsptr<const GridObject> object);

private:
	void CreateShaderVars();
	void ReleaseShaderVars();

	void CalculateFrustumPlanes();
};


/* Camera Component를 가지는 객체 */
class CameraObject : public Object {
	using base = Object;

protected:
	float mMovingSpeed{};
	sptr<Camera> mCamera{};

public:
	CameraObject();
	virtual ~CameraObject() = default;

	rsptr<Camera> GetCamera() const { return mCamera; }
	float GetMovingSpeed() const	{ return mMovingSpeed; }

	void SetMovingSpeed(float speed) { mMovingSpeed = speed; }

public:
	void Rotate(float pitch = 0.f, float yaw = 0.f, float roll = 0.f);

	void LookAt(const Vec3& lookAt, const Vec3& up);
};


/* main camera for scene rendering */
class MainCameraObject : public Singleton<MainCameraObject>, public CameraObject {
	friend class Singleton;

	using base = CameraObject;

private:
	MainCameraObject() = default;
	virtual ~MainCameraObject() = default;

public:
	virtual void Start() override;
};
#pragma endregion