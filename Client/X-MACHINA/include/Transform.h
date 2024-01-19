#pragma once

// position, rotation of an object.
// every object in a Scene has a Transform.
// it's used to store and manipulate the position, rotation of the object.
// every Transform can have a parent, child and sibling
// transform can access to self Object
class Transform {
private:
	Vec4x4 mWorldTransform = Matrix4x4::Identity();	// transform of     affected by a parent (world)
	Vec4x4 mLocalTransform = Matrix4x4::Identity();	// transform of not affected by a parent (local)
	Vec4x4 mPrevTransform  = Matrix4x4::Identity();	// transform of previous frame

	Vec3 mPosition = Vector3::Zero();		// local space position
	Vec3 mRight	   = Vector3::Right();		// right(x) axis in local space
	Vec3 mUp	   = Vector3::Up();			// up(y)    axis in local space
	Vec3 mLook	   = Vector3::Forrward();	// look(z)  axis in local space

	void* mObject{};	// self Object

	float mPitch{};		// euler angle of current x local rotation
	float mYaw{};		// euler angle of current y local rotation
	float mRoll{};		// euler angle of current z local rotation

protected:
	mutable bool	mUseObjCB{ false };	// 오브젝트 당 상수 버퍼를 사용하는지 확인하는 변수
	mutable int		mObjCBIdx{ -1 };	// 오브젝트 당 상수 버퍼에서 자신의 위치 처음에 유효하지 않은 값을 줘야한다.
	
public:
	Transform*		mParent{};
	sptr<Transform> mChild{};
	sptr<Transform> mSibling{};

public:
#pragma region C/Dtor
	// guarantee that transform can access to self Object
	template <typename T, typename = std::enable_if_t<std::is_base_of<Transform, T>::value>>
	Transform(T* object) { mObject = object; }

	virtual ~Transform() = default;
#pragma endregion

#pragma region Getter
	/* Position */
	// gets the position in world space 
	Vec3 GetPosition()      const	{ return Vec3(mWorldTransform._41, mWorldTransform._42, mWorldTransform._43); }
	// gets the position in local space 
	Vec3 GetLocalPosition() const	{ return mPosition; }

	/* Axis */
	// returns a right(x) axis in world space
	Vec3 GetRight() const			{ return Vector3::Normalize(Vec3(mWorldTransform._11, mWorldTransform._12, mWorldTransform._13)); }
	// returns a up(y) axis in world space
	Vec3 GetUp()    const			{ return Vector3::Normalize(Vec3(mWorldTransform._21, mWorldTransform._22, mWorldTransform._23)); }
	// returns a look(z) axis in world space
	Vec3 GetLook()  const			{ return Vector3::Normalize(Vec3(mWorldTransform._31, mWorldTransform._32, mWorldTransform._33)); }
	// [dwDirection]에 따른 이 Transform의 diretion을 반환한다.
	// [dwDirection]=Right&Front ==> return mRight + mLook
	Vec3 GetDirection(DWORD dwDirection, float distance = 1.f) const;

	/* Transform */
	const Vec4x4& GetWorldTransform() const { return mWorldTransform; }
	const Vec4x4& GetLocalTransform() const { return mLocalTransform; }
	float GetPitch() const					{ return mPitch; }
	float GetYaw()   const					{ return mYaw; }
	float GetRoll()  const					{ return mRoll; }

	/* Others */
	Transform* GetParent() const			{ return mParent; }

	/* ObjectCB Index */
	const int GetObjCBIdx() const			{ return mObjCBIdx; }
	const bool GetUseObjCB() const			{ return mUseObjCB; }

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
	// set axis from the [look], [up], [right] vectors
	void SetAxis(const Vec3& look, const Vec3& up, const Vec3& right);
	// set axis from [axisMatrix]
	void SetAxis(const Vec4x4& axisMatrix);

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
	void SetChild(rsptr<Transform> child);

	void SetLocalTransform(const Vec4x4& transform);

	void SetObjCBIdx(int val) const { mObjCBIdx = val; }
	void SetUseObjCB(bool val) const { mUseObjCB = val; }
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
	// (local) rotates around [offset] by [axis] and [angle]
	void RotateOffset(const Vec3& axis, float angle, const Vec3& offset);

	// (local) rotates to the [lookTo] direction
	void LookTo(const Vec3& lookTo, const Vec3& up = Vector3::Up());
	// (local) rotates the transform so the forward vector points at [lookAt] position.
	void LookAt(const Vec3& lookAt, const Vec3& up = Vector3::Up());

	/* Transform */
#pragma region Transform
public:
	void SetWorldTransform(const Vec4x4& transform);
	// set local transform to previous transform
	void ReturnToPrevTransform();

private:
	// set axis vectors from local transform
	void UpdateAxis(bool isComputeWorldTransform = true);
	// set local transform from axis vectors
	void UpdateLocalTransform(bool isComputeWorldTransform = true);

	// 부모의 world transform을 적용한 transform을 반환
	void RequestTransform(Vec4x4& transform);

	// 부모의 world transform과 내 local transform을 곱해 내 world transform을 계산한다.
	void ComputeWorldTransform(const Vec4x4* parentTransform = nullptr);

public:
#pragma endregion

	/* Others */
	virtual void Awake();
	virtual void OnDisable();
	
	// 오브젝트 상수 버퍼 인덱스를 반환
	void ReturnObjCBIdx();

	void BeforeUpdateTransform();
	virtual void UpdateTransform() { ComputeWorldTransform(); }

	virtual void UpdateShaderVars() const;


	void NormalizeAxis();

	// returns self Object
	template<class T>
	T* GetObj() { return static_cast<T*>(mObject); }
	template<class T>
	const T* GetObj() const { return static_cast<T*>(mObject); }

	// Merge all under transforms from [rootTransform] except a parent
	static void MergeTransform(std::vector<const Transform*>& out, const Transform* rootTransform);
	// set transform matrix (SetGraphicsRoot32BitConstants)
	static void UpdateShaderVars(const Matrix& matrix);
};