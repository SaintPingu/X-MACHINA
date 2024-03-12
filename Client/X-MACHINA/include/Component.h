#pragma once

#pragma region Include
#include "Transform.h"
#pragma endregion


#pragma region Define
#define COMPONENT_ABSTRACT( className, parent )							\
public:																	\
	className(Object* object) : parent(object) {}						\
	virtual ~className() = default;										\
																		\
private:																\
	using base = parent;												\

#define COMPONENT( className, parent )									\
public:																	\
    static const int ID = static_cast<int>(ComponentID::className);		\
    className(Object* object) : parent(object) { }						\
    virtual ~className() = default; 									\
	virtual int GetID() const override { return ID; }					\
																		\
private:																\
	using base = parent;												\

#pragma endregion


#pragma region ClassForwardDecl
class Object;
#pragma endregion


#pragma region EnumClass
enum class ComponentID {	// except abstract component
	BoxCollider = 0,
	SphereCollider,
	ObjectCollider,
	Camera,
	Rigidbody,
	ParticleSystem,

	/* scripts */
	Script_Apache,
	Script_Gunship,
	Script_MainCamera,
	Script_ExplosiveObject,
	Script_AirplanePlayer,
	Script_TankPlayer,
	Script_Fragment,
	Script_Bullet,
	Script_Billboard,
	Script_Sprite,
	Script_GroundPlayer,
	_count
};

enum class ObjectTag : DWORD {
	Unspecified    = 0x000,
	Player         = 0x001,
	Building       = 0x002,
	ExplosiveSmall = 0x004,
	ExplosiveBig   = 0x008,
	Tank           = 0x010,
	Helicopter     = 0x020,
	Environment	   = 0x040,
	Bullet         = 0x080,
	Billboard      = 0x100,
	Terrain        = 0x200,
	Water          = 0x400,
	Sprite         = 0x800,
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
protected:
	Object* mObject{};	// �� Component�� �����ϴ� ��ü

private:
	bool mIsActive = true;
	static constexpr int kNotID = -1;

public:
	Component(Object* object) { mObject = object; }
	virtual ~Component() = default;

	virtual int GetID() const { return kNotID; }
	bool IsActive() const { return mIsActive; }
	
	void SetActive(bool isActive) { mIsActive = isActive; }

public:
	// ���� �� �� ȣ��ȴ�.
	virtual void Awake() {}

	// ��ü Ȱ��ȭ �� ȣ��ȴ�.
	virtual void OnEnable() {}

	// ��ü ��Ȱ��ȭ �� ȣ��ȴ�.
	virtual void OnDisable() {}

	// Update ȣ�� �� �� �� ȣ��ȴ�.
	virtual void Start() {}

	// �� ������ ȣ��ȴ�.
	virtual void Update() {}

	// �� ������ ȣ��ȴ�.
	virtual void Animate() {}

	// ��ü�� �Ҹ�� �� ȣ��ȴ�.
	virtual void OnDestroy() {}

	// ���� ���ҽ��� �����Ѵ�.
	virtual void Release() {}

	virtual void ReleaseUploadBuffers() {}

	// ��ü(other)�� �浹 �� ȣ��ȴ�.
	virtual void OnCollisionStay(Object& other) {}
};





class Object : public Transform {
private:
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

	bool IsActive() const				{ return mIsEnable; }
#pragma endregion

#pragma region Setter
	void SetTag(ObjectTag tag);
	void SetName(const std::string& name)	{ mName = name; }
	void SetLayer(ObjectLayer layer)		{ mLayer = layer; }
#pragma endregion



#pragma region Component
	// �ֻ��� T component�� ��ȯ�Ѵ�.
	template<class T>
	sptr<T> GetComponent() const
	{
		for (const auto& component : mComponents) {
			if (component->GetID() == T::ID) {
				return std::static_pointer_cast<T>(component);
			}
		}

		return nullptr;
	}

	// ��� T component���� ��ȯ�Ѵ�.
	template<class T>
	std::vector<sptr<T>> GetComponents() const
	{
		std::vector<sptr<T>> result{};

		for (const auto& component : mComponents) {
			if (component->GetID() == T::ID) {
				result.emplace_back(std::static_pointer_cast<T>(component));
			}
		}

		return result;
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
		mComponents.emplace_back(std::make_shared<T>((Object*)this));
		return std::static_pointer_cast<T>(mComponents.back());
	}

	// �ֻ��� T component�� �����Ѵ�.
	template<class T>
	void RemoveComponent()
	{
		for (auto it = mComponents.begin(); it != mComponents.end(); ++it) {
			if ((*it)->GetID() == T::ID) {
				mComponents.erase(it);
				return;
			}
		}
	}

	// src ��ü�� ��� component���� ������ �߰��Ѵ�.
	void CopyComponents(const Object& src);
#pragma endregion

	// Awake -> OnEnable -> Start -> Update -> Animate
	virtual void Awake() override;
	virtual void OnEnable();
	virtual void OnDisable();
	virtual void Start();
	virtual void Update();
	virtual void Animate();
	virtual void OnDestroy();
	virtual void Release();
	virtual void ReleaseUploadBuffers();

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