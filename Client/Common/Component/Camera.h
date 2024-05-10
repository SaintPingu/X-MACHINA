#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region Define
#define MAIN_CAMERA MainCamera::I->GetCamera()
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
#pragma endregion

#pragma region Class

class Camera : public Component {
	COMPONENT(Camera, Component)

private:
	float mAspectRatio{};

	Matrix mViewTransform{};
	Matrix mProjTransform{};
	Matrix mNoLagViewTransform{};

	Vec3 mOffset{};

	BoundingFrustum mFrustumView{};
	BoundingFrustum mFrustumWorld{};
	BoundingFrustum mFrustumWorldShadow{};

	D3D12_VIEWPORT	mViewport{};
	D3D12_RECT		mScissorRect{};

public:
	Vec3 GetOffset() const { return mOffset; }
	Vec3 GetPosition() const { return mObject->GetPosition(); }
	Vec3 GetRight() const { return mObject->GetRight(); }
	Vec3 GetUp() const { return mObject->GetUp(); }
	const Matrix& GetViewMtx() const { return mViewTransform; }
	const Matrix& GetProjMtx() const { return mProjTransform; }
	const Matrix& GetNoLagViewtx() const { return mNoLagViewTransform; }
	float GetWidth() const { return mViewport.Width; }
	float GetHeight() const { return mViewport.Height; }

	void SetOffset(const Vec3& offset) { mOffset = offset; }

public:
	virtual void Awake() override;

	void UpdateViewMtx();
	void SetProjMtx(float nearPlaneDistance, float farPlaneDistance, float fovAngle);
	void SetNoLagViewMtx(const Matrix& mtx) { mNoLagViewTransform = mtx; }
		 
	void SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ = 0.f, float maxZ = 1.f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);
	void SetViewportsAndScissorRects();

	void LookAt(const Vec3& lookAt, const Vec3& up);

	bool IsInFrustum(const BoundingBox& boundingBox)		 { return mFrustumWorld.Intersects(boundingBox); }
	bool IsInFrustum(const BoundingOrientedBox& boundingBox) { return mFrustumWorld.Intersects(boundingBox); }
	bool IsInFrustum(const BoundingSphere& boundingSphere)   { return mFrustumWorld.Intersects(boundingSphere); }
	const BoundingFrustum& GetFrustum() const { return mFrustumWorld; }
	const BoundingFrustum& GetFrustumShadow() const { return mFrustumWorldShadow; }

	Vec2 WorldToScreenPoint(const Vec3& pos);
	Vec3 ScreenToWorldRay(const Vec2& pos);

	Vec2 ScreenToNDC(const Vec2& pos);

private:
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
class MainCamera : public Singleton<MainCamera>, public CameraObject {
	friend Singleton;

	using base = CameraObject;

private:
	MainCamera() = default;
	virtual ~MainCamera() = default;

public:
	virtual void Awake() override;
};
#pragma endregion