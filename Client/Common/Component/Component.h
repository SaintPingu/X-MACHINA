#pragma once

#pragma region Include
#include "Transform.h"
#pragma endregion

#pragma region Define

#define COMPONENT( className, parent )				\
public:												\
    className(Object* object) : parent(object) { }	\
private:											\
	using base = parent;

#define COMPONENT_ABSTRACT( className, parent )		\
public:												\
    className(Object* object) : parent(object) { }	\
private:											\
	using base = parent;

#pragma endregion

#pragma region ClassForwardDecl
class Object;
#pragma endregion


#pragma region Functions
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
	bool mIsActive = false;

	std::function<void()> UpdateFunc{ std::bind(&Component::FirstUpdate, this) };

public:
	Component(Object* object) { mObject = object; }
	virtual ~Component() = default;

	bool IsAwake() const { return mIsAwake; }
	bool IsStart() const { return mIsStart; }
	bool IsActive() const { return mIsActive; }

	void SetActive(bool isActive);

protected:
	// 최초 한 번 호출된다.
	virtual void Awake() { mIsAwake = true; }

	// 객체 활성화 시 호출된다.
	virtual void OnEnable()
	{
		if (!mIsAwake) {
			Awake();
		}

		mIsActive = true;
	}

	// 객체 비활성화 시 호출된다.
	virtual void OnDisable()
	{
		mIsActive = false;
	}

	// Update 호출 전 한 번 호출된다.
	virtual void Start()
	{
		mIsStart = true;
	}

	// 매 프레임 호출된다. (before aninate)
	virtual void Update() {}

	// 매 프레임 호출된다.
	virtual void Animate() {}

	// 매 프레임 호출된다. (after aninate)
	virtual void LateUpdate() {}

	// 객체가 소멸될 시 호출된다.
	virtual void OnDestroy()
	{
		SetActive(false);
	}

	// 동적 리소스를 해제한다.
	virtual void Release() {}

	// 객체(other)와 충돌 시 한 번 호출된다.
	virtual void OnCollisionEnter(Object& other) {}
	// 충돌 중인 객체(other)에 대해 매 프레임 호출된다.
	virtual void OnCollisionStay(Object& other) {}
	// 객체(other)와 충돌 종료 시 한 번 호출된다.
	virtual void OnCollisionExit(Object& other) {}

private:
	void FirstUpdate();
};





class Object : public Transform {
	friend class Scene;
	friend class Grid;
	friend class CollisionManager;
	friend class DynamicEnvironmentMappingManager;

private:
	UINT32			mID;
	static UINT32	sID;

	std::string	mName{};
	ObjectTag	mTag{};
	ObjectLayer mLayer{};
	ObjectType	mType{};

	bool mIsAwake  = false;			// Awake()가 호출되었는가?
	bool mIsStart  = false;			// Start()가 호출되었는가?
	bool mIsEnable = false;			// OnEnable()이 호출되었는가? (활성화 상태인가?)
	bool mIsDestroyed = false;		// Destroy()가 호출되었는가?

private:
	std::vector<sptr<Component>> mComponents{};
	std::unordered_set<Object*> mCollisionObjects{};	// 한 프레임에서 충돌한 오브젝트 집합

public:
#pragma region C/Dtor
	Object() : Transform(this) { mID = sID++; }
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

	const std::unordered_set<Object*>& GetCollisionObjects() { return mCollisionObjects; }
#pragma endregion

#pragma region Setter
	void SetTag(ObjectTag tag);
	void SetName(const std::string& name)	{ mName = name; }
	void SetLayer(ObjectLayer layer)		{ mLayer = layer; }
	void SetID(UINT32 sessionID)			{ mID = sessionID; }
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
	sptr<T> AddComponent(bool enable = true)
	{
		static_assert(!std::is_abstract<T>::value, "The component type must not be abstract when added.");

		sptr<T> component = std::make_shared<T>((Object*)this);
		mComponents.push_back(component);
		if (enable && IsActive()) {
			component->OnEnable();
		}

		return component;
	}

	// 최상위 T component를 제거한다.
	template<class T>
	void RemoveComponent()
	{
		for (auto it = mComponents.begin(); it != mComponents.end(); ++it) {
			auto& component = *it;
			auto result = std::dynamic_pointer_cast<T>(component);
			if (result) {
				result->OnDestroy();
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
			const auto& result = std::dynamic_pointer_cast<T>(component)
			if (result) {
				result->OnDestroy();
				return true;
			}
			return false;
			});
	}


	// src 객체의 모든 component들을 복사해 추가한다.
	void CopyComponents(const Object& src);
#pragma endregion

	// [frameName]의 Transform을 계층 구조에서 찾아 반환한다 (없으면 nullptr)
	Transform* FindFrame(const std::string& frameName);

	void SetActive(bool isActive);

	void Destroy()
	{
		if (!mIsDestroyed) {
			OnDestroy();
		}
	}

	// 충돌한 객체(other) 모두에 대해 모든 component의 OnCollisioStay 함수를 매 프레임 호출한다.
	virtual void OnCollisionStay();

	bool IsCollided(Object* other) const { return mCollisionObjects.count(other); }

protected:
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

	// 객체(other)와 충돌 시 한 번 호출된다.
	virtual void OnCollisionEnter(Object& other);
	// 객체(other)와 충돌 종료 시 한 번 호출된다.
	virtual void OnCollisionExit(Object& other);

private:
	// 모든 component들에 대해 (processFunc) 함수를 실행한다.
	void ProcessComponents(std::function<void(rsptr<Component>)> processFunc);

	// (component)의 복사본을 반환한다.
	sptr<Component> GetCopyComponent(rsptr<Component> component);
};
#pragma endregion