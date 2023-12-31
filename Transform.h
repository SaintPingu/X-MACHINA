#pragma once


class Transform {
private:
	std::string mClassType;

	Vec4x4 mWorldTransform	{ Matrix4x4::Identity() };
	Vec4x4 mLocalTransform	{ Matrix4x4::Identity() };
	Vec4x4 mPrevTransform	{ Matrix4x4::Identity() };

	Vec3 mPosition	{ Vector3::Zero() };
	Vec3 mRight		{ Vector3::Right() };
	Vec3 mUp		{ Vector3::Up() };
	Vec3 mLook		{ Vector3::Forrward() };
	Vec3 mScale		{ Vector3::One() };

	void* mObject{};

	float mPitch{};
	float mYaw{};
	float mRoll{};

	bool isUpdated{};

public:
	Transform*		mParent{};
	sptr<Transform> mChild{};
	sptr<Transform> mSibling{};

public:
#pragma region C/Dtor
	Transform() = default;
	Transform(void* object) { mObject = object; }
	virtual ~Transform() = default;
#pragma endregion

#pragma region Getter
	/* Position */
	Vec3 GetPosition() const { return Vec3(mWorldTransform._41, mWorldTransform._42, mWorldTransform._43); }
	Vec3 GetLocalPosition() const { return mPosition; }

	/* Axis */
	Vec3 GetLook() const { return Vector3::Normalize(Vec3(mWorldTransform._31, mWorldTransform._32, mWorldTransform._33)); }
	Vec3 GetUp() const { return Vector3::Normalize(Vec3(mWorldTransform._21, mWorldTransform._22, mWorldTransform._23)); }
	Vec3 GetRight() const { return Vector3::Normalize(Vec3(mWorldTransform._11, mWorldTransform._12, mWorldTransform._13)); }
	Vec3 GetDirection(DWORD dwDirection, float distance = 1.f) const;

	/* Transform */
	const Vec4x4& GetWorldTransform() const { return mWorldTransform; }
	const Vec4x4& GetLocalTransform() const { return mLocalTransform; }
	float GetPitch() const { return mPitch; }
	float GetYaw() const { return mYaw; }
	float GetRoll() const { return mRoll; }

	/* Others */
	Transform* GetParent() const { return mParent; }
#pragma endregion

#pragma region Setter
	/* Position */
	void SetPosition(float x, float y, float z);
	void SetPosition(const Vec3& pos);
	void SetPosition(const Vector& pos);

	void SetPositionX(float x);
	void SetPositionY(float y);
	void SetPositionZ(float z);

	/* Axis */
	void SetAxis(const Vec3& look, const Vec3& up, const Vec3& right);

	void SetRight(const Vec3& right);
	void SetLook(const Vec3& look);
	void SetUp(const Vec3& up);

	void SetRightX(float val) { mRight.x = val; SetRight(mRight); }
	void SetRightY(float val) { mRight.y = val; SetRight(mRight); }
	void SetRightZ(float val) { mRight.z = val; SetRight(mRight); }

	void SetLookX(float val) { mLook.x = val; SetLook(mLook); }
	void SetLookY(float val) { mLook.y = val; SetLook(mLook); }
	void SetLookZ(float val) { mLook.z = val; SetLook(mLook); }

	void SetUpX(float val) { mUp.x = val; SetUp(mUp); }
	void SetUpY(float val) { mUp.y = val; SetUp(mUp); }
	void SetUpZ(float val) { mUp.z = val; SetUp(mUp); }

	/* Others */
	void SetChild(rsptr<Transform> child);

	void SetTransform(const Vec4x4& transform);
#pragma endregion

public:
	/* Translate */
	void Translate(const Vec3& translation);
	void Translate(const Vec3& direction, float distance);
	void Translate(float x, float y, float z);

	/* Movement */
	void MoveLocal(const Vec3& translation);
	void MoveStrafe(float fDistance = 1.f);
	void MoveUp(float fDistance = 1.f);
	void MoveForward(float fDistance = 1.f);

	/* Rotation */
	void Rotate(float pitch = 0.f, float yaw = 0.f, float roll = 0.f);
	void Rotate(const Vec3& axis, float angle);
	void RotateOffset(const Vec3& axis, float angle, const Vec3& offset);

	void LookTo(const Vec3& lookTo, const Vec3& up);
	void LookAt(const Vec3& lookAt, const Vec3& up);

	/* Transform */
#pragma region Transform
public:
	void SetWorldTransform(const Vec4x4& transform);
	void ReturnTransform();

private:
	void UpdateAxis(bool isComputeWorldTransform = true);
	void UpdateTransform(bool isComputeWorldTransform = true);

	// 부모의 worldTransform을 적용한 transform을 반환
	void RequestTransform(Vec4x4& transform);

	void ComputeWorldTransform(const Vec4x4* parentTransform = nullptr);

public:
#pragma endregion

	/* Others */
	virtual void Update();

	virtual void UpdateShaderVars() const;

	void NormalizeAxis();

	template<class T>
	T* GetObj() { return static_cast<T*>(mObject); }
	template<class T>
	const T* GetObj() const { return static_cast<T*>(mObject); }

	// Merge all transforms except parent
	static void MergeTransform(std::vector<const Transform*>& out, const Transform* rootTransform);

	static void UpdateShaderVars(const Matrix& matrix);
};