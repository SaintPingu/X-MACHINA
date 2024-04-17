#pragma once

#pragma region Include
#include "Transform.h"
#pragma endregion

#pragma region Define

#define COMPONENT( className, parent )				\
public:												\
    className(Object* object) : parent(object) { }	\
	static bool IsAbstract() { return false; }		\
private:											\
	using base = parent;

#define COMPONENT_ABSTRACT( className, parent )		\
public:												\
    className(Object* object) : parent(object) { }	\
	static bool IsAbstract() { return true; }		\
private:											\
	using base = parent;

#pragma endregion

#pragma region ClassForwardDecl
class Object;
#pragma endregion

#pragma region EnumClass
class ObjectTag : public DwordOverloader<ObjectTag> {
	DWORD_OVERLOADER(ObjectTag)

	static const DWORD Unspecified    = 0x0000;
	static const DWORD Player         = 0x0001;
	static const DWORD Building       = 0x0002;
	static const DWORD ExplosiveSmall = 0x0004;
	static const DWORD ExplosiveBig   = 0x0008;
	static const DWORD Tank           = 0x0010;
	static const DWORD Helicopter     = 0x0020;
	static const DWORD Environment    = 0x0040;
	static const DWORD Bullet         = 0x0080;
	static const DWORD Billboard      = 0x0100;
	static const DWORD Terrain        = 0x0200;
	static const DWORD Water          = 0x0400;
	static const DWORD Sprite         = 0x0800;
	static const DWORD Enemy          = 0x1000;
};

// rendering layer
enum class ObjectLayer {
	Default = 0,
	Transparent,	// use transparent shader
	Water			// use water shader
};

// update type
enum class ObjectType {
	Static = 0,		// do not update, do not grid update.
	Env,			// also static,   but no collision.
	Dynamic,		// do update,	  do not grid update.
	DynamicMove,	// do update,     do     grid update.
};
#pragma endregion


#pragma region Function
// 유니티의 tag 문자열을 ObjectTag로 변환한다.
ObjectTag GetTagByString(const std::string& tag);

// 유니티의 Layer 번호[num]를 ObjectLayer로 변환한다.
ObjectLayer GetLayerByNum(int num);

// ObjectTag에 따른 ObjectType을 반환한다.
ObjectType GetObjectType(ObjectTag tag);
#pragma endregion


#pragma region Class
class Component {
	friend Object;

protected:
	Object* mObject{};	// 이 Component를 소유하는 객체

private:
	bool mIsAwake  = false;
	bool mIsStart  = false;
	bool mIsActive = true;

	std::function<void()> UpdateFunc{ std::bind(&Component::FirstUpdate, this) };

public:
	Component(Object* object) { mObject = object; }
	virtual ~Component() = default;

	bool IsActive() const { return mIsActive; }
	
	void SetActive(bool isActive)
	{
		if (isActive) {
			OnEnable();
		}
		else {
			OnDisable();
		}
	}

public:
	// 최초 한 번 호출된다.
	virtual void Awake() { mIsAwake = true; }

	// 객체 활성화 시 호출된다.
	virtual void OnEnable() { mIsActive = true; }

	// 객체 비활성화 시 호출된다.
	virtual void OnDisable() { mIsActive = false; }

	// Update 호출 전 한 번 호출된다.
	virtual void Start() { mIsStart = true; }

	// 매 프레임 호출된다. (before aninate)
	virtual void Update() {}

	// 매 프레임 호출된다.
	virtual void Animate() {}

	// 매 프레임 호출된다. (after aninate)
	virtual void LateUpdate() {}

	// 객체가 소멸될 시 호출된다.
	virtual void OnDestroy() {}

	// 동적 리소스를 해제한다.
	virtual void Release() {}

	// 객체(other)와 충돌 시 호출된다.
	virtual void OnCollisionStay(Object& other) {}

private:
	void FirstUpdate();
};





class Object : public Transform {
private:
	UINT32      mID;

	std::string	mName{};
	ObjectTag	mTag{};
	ObjectLayer mLayer{};
	ObjectType	mType{};

	bool mIsAwake  = false;			// Awake()가 호출되었는가?
	bool mIsStart  = false;			// Start()가 호출되었는가?
	bool mIsEnable = false;			// OnEnable()이 호출되었는가? (활성화 상태인가?)

private:
	std::vector<sptr<Component>> mComponents{};
	std::unordered_set<const Object*> mCollisionObjects{};	// 한 프레임에서 충돌한 오브젝트 집합

public:
#pragma region C/Dtor
	Object() : Transform(this) { }
	virtual ~Object() { Release(); }
#pragma endregion

#pragma region Getter
	const std::string& GetName() const	{ return mName; }
	ObjectTag GetTag() const			{ return mTag; }
	ObjectLayer GetLayer() const		{ return mLayer; }
	ObjectType GetType() const			{ return mType; }
	UINT32 GetID() const				{ return mID; }

	bool IsAwake() const				{ return mIsAwake; }
	bool IsStart() const				{ return mIsStart; }
	bool IsActive() const				{ return mIsEnable; }
#pragma endregion

#pragma region Setter
	void SetTag(ObjectTag tag);
	void SetName(const std::string& name)	{ mName = name; }
	void SetLayer(ObjectLayer layer)		{ mLayer = layer; }
	void setID(UINT32 sessionID)			{ mID = sessionID; }
#pragma endregion



#pragma region Component
	// 최상위 T component를 반환한다.
	template<class T>
	sptr<T> GetComponent() const
	{
		for (const auto& component : mComponents) {
			auto result = std::dynamic_pointer_cast<T>(component);
			if (result) {
				return result;
			}
		}

		return nullptr;
	}

	template<class T>
	std::vector<sptr<T>> GetComponents() const
	{
		std::vector<sptr<T>> results{};

		for (const auto& component : mComponents) {
			auto result = std::dynamic_pointer_cast<T>(component);
			if (result) {
				results.push_back(result);
			}
		}

		return results;
	}

	// 모든 component들을 반환한다.
	const std::vector<sptr<Component>>& GetAllComponents() const
	{
		return mComponents;
	}

	// T component를 추가한다.
	template<class T>
	sptr<T> AddComponent()
	{
		if (T::IsAbstract()) {
			// log here : Cannot add component of this type because it is abstract.
			return nullptr;
		}
		mComponents.emplace_back(std::make_shared<T>((Object*)this));
		return std::static_pointer_cast<T>(mComponents.back());
	}

	// 최상위 T component를 제거한다.
	template<class T>
	void RemoveComponent()
	{
		for (auto it = mComponents.begin(); it != mComponents.end(); ++it) {
			auto component = *it;
			auto result = std::dynamic_pointer_cast<T>(component);
			if (result) {
				mComponents.erase(it);
				return;
			}
		}
	}

	// 모든 T component를 제거한다.
	template<class T>
	void RemoveAllComponents()
	{
		std::erase_if(mComponents, [](const auto& component) {
			return std::dynamic_pointer_cast<T>(component);
			});
	}


	// src 객체의 모든 component들을 복사해 추가한다.
	void CopyComponents(const Object& src);
#pragma endregion

	// Awake -> OnEnable -> Start -> Update -> Animate -> LateUpdate
	virtual void Awake() override;
	virtual void OnEnable();
	virtual void OnDisable();
	virtual void Start();
	virtual void Update();
	virtual void Animate();
	virtual void LateUpdate();
	virtual void OnDestroy();
	virtual void Release();

	// 객체(other)와 충돌 시 호출된다.
	virtual void OnCollisionStay(Object& other);

	// [frameName]의 Transform을 계층 구조에서 찾아 반환한다 (없으면 nullptr)
	Transform* FindFrame(const std::string& frameName);

private:
	// 모든 component들에 대해 (processFunc) 함수를 실행한다.
	void ProcessComponents(std::function<void(rsptr<Component>)> processFunc);

	// (component)의 복사본을 반환한다.
	sptr<Component> GetCopyComponent(rsptr<Component> component);
};
#pragma endregion