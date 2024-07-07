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
// ObjectTag�� ���� ObjectType�� ��ȯ�Ѵ�.
ObjectType GetObjectType(ObjectTag tag);
#pragma endregion


#pragma region Class
class Component {
	friend Object;

protected:
	Object* mObject{};	// �� Component�� �����ϴ� ��ü

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
	// ���� �� �� ȣ��ȴ�.
	virtual void Awake() { mIsAwake = true; }

	// ��ü Ȱ��ȭ �� ȣ��ȴ�.
	virtual void OnEnable()
	{
		if (!mIsAwake) {
			Awake();
		}

		mIsActive = true;
	}

	// ��ü ��Ȱ��ȭ �� ȣ��ȴ�.
	virtual void OnDisable()
	{
		mIsActive = false;
	}

	// Update ȣ�� �� �� �� ȣ��ȴ�.
	virtual void Start()
	{
		mIsStart = true;
	}

	// �� ������ ȣ��ȴ�. (before aninate)
	virtual void Update() {}

	// �� ������ ȣ��ȴ�.
	virtual void Animate() {}

	// �� ������ ȣ��ȴ�. (after aninate)
	virtual void LateUpdate() {}

	// ��ü�� �Ҹ�� �� ȣ��ȴ�.
	virtual void OnDestroy()
	{
		SetActive(false);
	}

	// ���� ���ҽ��� �����Ѵ�.
	virtual void Release() {}

	// ��ü(other)�� �浹 �� �� �� ȣ��ȴ�.
	virtual void OnCollisionEnter(Object& other) {}
	// �浹 ���� ��ü(other)�� ���� �� ������ ȣ��ȴ�.
	virtual void OnCollisionStay(Object& other) {}
	// ��ü(other)�� �浹 ���� �� �� �� ȣ��ȴ�.
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

	bool mIsAwake  = false;			// Awake()�� ȣ��Ǿ��°�?
	bool mIsStart  = false;			// Start()�� ȣ��Ǿ��°�?
	bool mIsEnable = false;			// OnEnable()�� ȣ��Ǿ��°�? (Ȱ��ȭ �����ΰ�?)
	bool mIsDestroyed = false;		// Destroy()�� ȣ��Ǿ��°�?

private:
	std::vector<sptr<Component>> mComponents{};
	std::unordered_set<Object*> mCollisionObjects{};	// �� �����ӿ��� �浹�� ������Ʈ ����

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
	// �ֻ��� T component�� ��ȯ�Ѵ�.
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

	// ��� component���� ��ȯ�Ѵ�.
	const std::vector<sptr<Component>>& GetAllComponents() const
	{
		return mComponents;
	}

	// T component�� �߰��Ѵ�.
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

	// �ֻ��� T component�� �����Ѵ�.
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

	// ��� T component�� �����Ѵ�.
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


	// src ��ü�� ��� component���� ������ �߰��Ѵ�.
	void CopyComponents(const Object& src);
#pragma endregion

	// [frameName]�� Transform�� ���� �������� ã�� ��ȯ�Ѵ� (������ nullptr)
	Transform* FindFrame(const std::string& frameName);

	void SetActive(bool isActive);

	void Destroy()
	{
		if (!mIsDestroyed) {
			OnDestroy();
		}
	}

	// �浹�� ��ü(other) ��ο� ���� ��� component�� OnCollisioStay �Լ��� �� ������ ȣ���Ѵ�.
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

	// ��ü(other)�� �浹 �� �� �� ȣ��ȴ�.
	virtual void OnCollisionEnter(Object& other);
	// ��ü(other)�� �浹 ���� �� �� �� ȣ��ȴ�.
	virtual void OnCollisionExit(Object& other);

private:
	// ��� component�鿡 ���� (processFunc) �Լ��� �����Ѵ�.
	void ProcessComponents(std::function<void(rsptr<Component>)> processFunc);

	// (component)�� ���纻�� ��ȯ�Ѵ�.
	sptr<Component> GetCopyComponent(rsptr<Component> component);
};
#pragma endregion