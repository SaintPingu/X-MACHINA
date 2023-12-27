#pragma once
#include "Component.h"

#define ASPECT_RATIO				(float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))


class CGameObject;


struct VS_CB_CAMERA_INFO
{
	Vec4x4 mView{};
	Vec4x4 mProjection{};
	Vec3 mPosition{};
};












class Camera : public Component {
	COMPONENT(Component, Camera)

private:
	Transform* mLocalTransform{};

protected:
	ComPtr<ID3D12Resource> mCbCamera{};

	VS_CB_CAMERA_INFO* mCbMappedCamera{};
protected:
	DWORD mCamMode{};

	Vec3 mLookAtWorld{};
	Vec3 mOffset{};

	Vec4x4 mViewTransform = Matrix4x4::Identity();
	Vec4x4 mProjectionTransform = Matrix4x4::Identity();

	BoundingFrustum mFrustumView{};
	BoundingFrustum mFrustumWorld{};

	D3D12_VIEWPORT mViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	D3D12_RECT mScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };

public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Constructor ] /////

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Getter ] /////

	DWORD GetMode() { return(mCamMode); }

	Vec3& GetLookAtPosition() { return(mLookAtWorld); }
	Vec3 GetLook() const { return mLocalTransform->GetLook(); }

	/*float& GetPitch() { return(mPitch); }
	float& GetRoll() { return(mRoll); }
	float& GetYaw() { return(mYaw); }*/

	Vec3& GetOffset() { return(mOffset); }
	Vec3 GetPosition();

	Vec4x4 GetViewMatrix() { return(mViewTransform); }
	Vec4x4 GetProjectionMatrix() { return(mProjectionTransform); }

	D3D12_VIEWPORT GetViewport() { return(mViewport); }
	D3D12_RECT GetScissorRect() { return(mScissorRect); }

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Setter ] /////

	void SetMode(DWORD nMode) { mCamMode = nMode; }

	void SetLookAtPosition(Vec3 lookAtWorld) { mLookAtWorld = lookAtWorld; }

	void SetOffset(Vec3 xmf3Offset) { mOffset = xmf3Offset; }

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Others ] /////

	/* Component */
	virtual void Start() override;
	virtual void Update() override;

	/* Shader Variables */
	virtual void CreateShaderVariables();
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables();

	/* View Matrix */
	void GenerateViewMatrix();
	void RegenerateViewMatrix();
	void GenerateProjectionMatrix(float nearPlaneDistance, float farPlaneDistance, float aspectRatio, float fovAngle);

	/* Viewport */
	void SetViewport(int xTopLeft, int yTopLeft, int width, int height, float minZ = 0.0f, float maxZ = 1.0f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);

	virtual void SetViewportsAndScissorRects();

	/* Transform */
	void LookAt(const Vec3& lookAt, const Vec3& up);

	/* Frustum */
	void CalculateFrustumPlanes();
	bool IsInFrustum(const BoundingBox& boundingBox);
	bool IsInFrustum(const BoundingOrientedBox& boundingBox);
	bool IsInFrustum(const BoundingSphere& boundingSphere);
	bool IsInFrustum(rsptr<const CGameObject> object);
	
};






class CCameraObject : public CObject {
protected:
	float mMovingSpeed{};
	sptr<Camera> mCamera{};

public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Constructor ] /////

	CCameraObject();
	virtual ~CCameraObject() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Getter ] /////

	rsptr<Camera> GetCamera() const { return mCamera; }
	float GetMovingSpeed() const { return mMovingSpeed; }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Setter ] /////

	void SetMovingSpeed(float speed) { mMovingSpeed = speed; }


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Others ] /////

	/* CObject */
	virtual void Start() override;
	virtual void Update() override { CObject::Update(); }

	/* Shader Variables */
	virtual void CreateShaderVariables();
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables();

	/* Transform */
	void Rotate(float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f);
	void MoveLocal(const Vec3& xmf3Shift);

	/* Camera */
	virtual void LookAt(const Vec3& lookAt, const Vec3& up);
};






class CMainCamera : public CCameraObject {
private:
	CGameObject* mPlayer{};

public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Constructor ] /////

	CMainCamera() = default;
	virtual ~CMainCamera() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Getter ] /////

	Vec3 GetPlayerPos() const;
	CGameObject* GetPlayer() const { return mPlayer; }


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Setter ] /////

	void SetPlayer(rsptr<CGameObject> player) { mPlayer = player.get(); }


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Others ] /////

	/* CObject */
	virtual void Start() override;
	virtual void Update() override;

	/* Camera */
	virtual void LookAt(const Vec3& lookAt);
	virtual void LookPlayer();
};