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
// ����Ƽ�� tag ���ڿ��� ObjectTag�� ��ȯ�Ѵ�.
ObjectTag GetTagByString(const std::string& tag);

// ����Ƽ�� Layer ��ȣ[num]�� ObjectLayer�� ��ȯ�Ѵ�.
ObjectLayer GetLayerByNum(int num);

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
	// ���� �� �� ȣ��ȴ�.
	virtual void Awake() { mIsAwake = true; }

	// ��ü Ȱ��ȭ �� ȣ��ȴ�.
	virtual void OnEnable() { mIsActive = true; }

	// ��ü ��Ȱ��ȭ �� ȣ��ȴ�.
	virtual void OnDisable() { mIsActive = false; }

	// Update ȣ�� �� �� �� ȣ��ȴ�.
	virtual void Start() { mIsStart = true; }

	// �� ������ ȣ��ȴ�. (before aninate)
	virtual void Update() {}

	// �� ������ ȣ��ȴ�.
	virtual void Animate() {}

	// �� ������ ȣ��ȴ�. (after aninate)
	virtual void LateUpdate() {}

	// ��ü�� �Ҹ�� �� ȣ��ȴ�.
	virtual void OnDestroy() {}

	// ���� ���ҽ��� �����Ѵ�.
	virtual void Release() {}

	// ��ü(other)�� �浹 �� ȣ��ȴ�.
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

	bool mIsAwake  = false;			// Awake()�� ȣ��Ǿ��°�?
	bool mIsStart  = false;			// Start()�� ȣ��Ǿ��°�?
	bool mIsEnable = false;			// OnEnable()�� ȣ��Ǿ��°�? (Ȱ��ȭ �����ΰ�?)

private:
	std::vector<sptr<Component>> mComponents{};
	std::unordered_set<const Object*> mCollisionObjects{};	// �� �����ӿ��� �浹�� ������Ʈ ����

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
	sptr<T> AddComponent()
	{
		if (T::IsAbstract()) {
			// log here : Cannot add component of this type because it is abstract.
			return nullptr;
		}
		mComponents.emplace_back(std::make_shared<T>((Object*)this));
		return std::static_pointer_cast<T>(mComponents.back());
	}

	// �ֻ��� T component�� �����Ѵ�.
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

	// ��� T component�� �����Ѵ�.
	template<class T>
	void RemoveAllComponents()
	{
		std::erase_if(mComponents, [](const auto& component) {
			return std::dynamic_pointer_cast<T>(component);
			});
	}


	// src ��ü�� ��� component���� ������ �߰��Ѵ�.
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

	// ��ü(other)�� �浹 �� ȣ��ȴ�.
	virtual void OnCollisionStay(Object& other);

	// [frameName]�� Transform�� ���� �������� ã�� ��ȯ�Ѵ� (������ nullptr)
	Transform* FindFrame(const std::string& frameName);

private:
	// ��� component�鿡 ���� (processFunc) �Լ��� �����Ѵ�.
	void ProcessComponents(std::function<void(rsptr<Component>)> processFunc);

	// (component)�� ���纻�� ��ȯ�Ѵ�.
	sptr<Component> GetCopyComponent(rsptr<Component> component);
};
#pragma endregion