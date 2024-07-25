#pragma once

// position, rotation of an object.
// every object in a Scene has a Transform.
// it's used to store and manipulate the position, rotation of the object.
// every Transform can have a parent, child and sibling
// transform can access to self Object
class Transform : public std::enable_shared_from_this<Transform> {
private:
	int mIndex{};
	Matrix mWorldTransform{};	// transform of     affected by a parent (world)
	Matrix mLocalTransform{};	// transform of not affected by a parent (local)
	Matrix mPrevTransform {};	// transform of previous frame

	Vec3 mPosition = Vector3::Zero;		// local space position
	Vec3 mRight	   = Vector3::Right;	// right(x) axis in local space
	Vec3 mUp	   = Vector3::Up;		// up(y)    axis in local space
	Vec3 mLook	   = Vector3::Forward;	// look(z)  axis in local space

	void* mObject{};	// self Object

protected:
	mutable bool				mUseObjCB{};		// 오브젝트 상수 버퍼 사용 플래그
	mutable int					mObjCBCount{};		// 몇 개의 오브젝트 인덱스를 사용하였는가
	mutable std::vector<int>	mObjCBIndices{};	// 서브 메쉬를 가진 객체는 여러 개의 인덱스들을 가져야 한다.

public:
	mutable ObjectConstants		mObjectCB{};

public:
	Transform*					mParent{};
	sptr<Transform>				mChild{};
	sptr<Transform>				mSibling{};

public:
#pragma region C/Dtor
	// guarantee that transform can access to self Object
	template <typename T, typename = std::enable_if_t<std::is_base_of<Transform, T>::value>>
	Transform(T* object) : mObjCBIndices(30, -1) { mObject = object; }

	Transform& operator=(const Transform& other) {
		mWorldTransform = other.mWorldTransform;
		mLocalTransform = other.mLocalTransform;
		mPrevTransform = other.mPrevTransform;
		mPosition = other.mPosition;
		mRight = other.mRight;
		mUp = other.mUp;
		mLook = other.mLook;

		return *this;
	}

	virtual ~Transform() = default;
#pragma endregion

#pragma region Getter
	sptr<Transform> GetShared() { return shared_from_this(); }
	/* Position */
	// gets the position in world space 
	Vec3 GetPosition()      const			{ return Vec3(mWorldTransform._41, mWorldTransform._42, mWorldTransform._43); }
	// gets the position in local space 
	const Vec3& GetLocalPosition() const	{ return mPosition; }

	/* Axis */
	// returns a right(x) axis in world space
	Vec3 GetRight() const	{ return Vector3::Normalized(Vec3(mWorldTransform._11, mWorldTransform._12, mWorldTransform._13)); }
	// returns a up(y) axis in world space
	Vec3 GetUp()    const	{ return Vector3::Normalized(Vec3(mWorldTransform._21, mWorldTransform._22, mWorldTransform._23)); }
	// returns a look(z) axis in world space
	Vec3 GetLook()  const	{ return Vector3::Normalized(Vec3(mWorldTransform._31, mWorldTransform._32, mWorldTransform._33)); }
	// returns a quaternion in local space
	Quat GetLocalRotation() const;
	// returns a quaternion in world space
	Quat GetRotation() const;
	// [dir]에 따른 이 Transform의 diretion 단위벡터를 반환한다.
	// [dir]=Right&Front ==> return mRight + mLook
	Vec3 GetDirection(Dir dir, float distance = 1.f) const;
	// [dir]에 따른 월드 공간 diretion 단위벡터를 반환한다.
	static Vec3 GetWorldDirection(Dir dir);

	float GetYAngle() const;

	/* Transform */
	const Matrix& GetWorldTransform() const { return mWorldTransform; }
	const Matrix& GetLocalTransform() const { return mLocalTransform; }

	/* ObjectCB Index */
	bool GetUseObjCB() const				{ return mUseObjCB; }
	int GetObjCBIndex(int index = 0) const	{ return mObjCBIndices[index]; }

#pragma endregion

#pragma region Setter
	/* Position */
	void SetPosition(float x, float y, float z);
	void SetPosition(const Vec2& pos);
	void SetPosition(const Vec3& pos);

	void SetPositionX(float x);
	void SetPositionY(float y);
	void SetPositionZ(float z);

	/* Axis */
	// set axis from the [look], [up], [right] vectors
	void SetAxis(const Vec3& look, const Vec3& up, const Vec3& right);
	// set axis from [axisMatrix]
	void SetAxis(const Matrix& axisMatrix);

	void SetRight(const Vec3& right);
	void SetLook (const Vec3& look);
	void SetUp   (const Vec3& up);

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
	// set sibiling if already has a child
	void SetChild(rsptr<Transform> child, bool isKeepLocalTransform = true);
	// return itself
	sptr<Transform> DetachParent(bool isKeepLocalTransform = true);

	void SetLocalTransform(const Matrix& transform, bool isComputeWorldTransform = true);

	// 상수 버퍼 인덱스를 한 번이라도 설정 하였다면 오브젝트 카운트는 최소 1이다.
	void SetObjCBIndex(int val, int index = 0) const { mObjCBIndices[index] = val; SetUseObjCB(true); }
	void SetUseObjCB(bool val) const { mUseObjCB = val; mObjCBCount = 1; }
#pragma endregion

public:
	/* Translate */
	// moves the transform in the direction and distance of [translation]
	void Translate(const Vec3& translation);
	// moves the transform in the [direction] and [distance]
	void Translate(const Vec3& direction, float distance);
	// moves the transform in the [x], [y], and [z]
	void Translate(float x, float y, float z);

	/* Movement */
	// moves the local transform in the direction and distance of [translation]
	void MoveLocal  (const Vec3& translation);
	// move in the local Right direction by a distance
	void MoveStrafe (float distance = 1.f);
	// move in the local Up direction by a distance
	void MoveUp     (float distance = 1.f);
	// move in the local Look direction by a distance
	void MoveForward(float distance = 1.f);

	/* Rotation */
	// (local) rotates by [pitch], [yaw], and [roll]
	void Rotate(float pitch = 0.f, float yaw = 0.f, float roll = 0.f);
	// (local) rotates by [axis] and [angle]
	void Rotate(const Vec3& axis, float angle);
	// (local) rotates by [quaternion]
	void Rotate(const Vec4 quaternion);
	// (local) rotates to dir
	void RotateToDir(const Vec3& dir);
	// (global) rotates by [axis] and [angle]
	void RotateGlobal(const Vec3& axis, float angle);
	// (global) rotates by [eulerAngles]
	void RotateGlobal(const Vec3& eulerAngles);
	// (local) rotates around [offset] by [axis] and [angle]
	void RotateOffset(const Vec3& axis, float angle, const Vec3& offset);
	// (local) rotates to target by Y-axis 
	bool RotateTargetAxisY(const Vec3& target, float rotationSpeed);
	// (local) set rotation to quaternion
	void SetLocalRotation(const Vec4& quaternion);
	void ResetRotation(float yAngle);

	// (local) rotates to the [lookTo] direction
	void LookTo(const Vec3& lookTo, const Vec3& up = Vector3::Up);
	// (local) rotates the transform so the forward vector points at [lookAt] position
	void LookAt(const Vec3& lookAt, const Vec3& up = Vector3::Up);
	// (world) rotates the transform so the forward vector look to the [lookTo] direction
	void LookToWorld(const Vec3& lookTo, const Vec3& up = Vector3::Up);
	// (world) rotates the transform so the "look" vector look to the [lookTo] direction
	void LookToWorld2(const Vec3& lookTo, const Vec3& look, const Vec3& up = Vector3::Up);

	/* Transform */
#pragma region Transform
public:
	void SetWorldTransform(const Matrix& transform);
	// set local transform to previous transform
	void ReturnToPrevTransform();

	// 부모의 world transform과 내 local transform을 곱해 내 world transform을 계산한다.
	void ComputeWorldTransform(const Matrix* parentTransform = nullptr);

private:
	// set axis vectors from local transform
	void UpdateAxis(bool isComputeWorldTransform = true);
	// set local transform from axis vectors
	void UpdateLocalTransform(bool isComputeWorldTransform = true);

public:
#pragma endregion

	/* Others */
	virtual void Awake();
	virtual void OnDestroy();
	
	// 오브젝트 상수 버퍼 인덱스를 반환
	void ReturnObjCBIndex();

	void BeforeUpdateTransform();

	virtual void UpdateShaderVars(const ObjectConstants& objectCB, const int cnt = 0) const;
	virtual void UpdateShaderVars(const int cnt = 0, const int matIndex = 0) const;

	void NormalizeAxis();

	void DoAllTransforms(const std::function<void(Transform*)>& processFunc);
	void DoAllChilds(const std::function<void(Transform*)>& processFunc);

	// returns self Object
	template<class T>
	T* GetObj() { return static_cast<T*>(mObject); }
	template<class T>
	const T* GetObj() const { return static_cast<T*>(mObject); }

	// Merge all under transforms from [rootTransform] except a parent
	static void MergeTransform(std::vector<const Transform*>& out, const Transform* rootTransform);
	// set transform matrix (SetGraphicsRoot32BitConstants)
	static void UpdateColliderShaderVars(const Matrix& matrix, const Vec4& color = Vec4{1.f, 0.f, 0.f, 1.f});
};