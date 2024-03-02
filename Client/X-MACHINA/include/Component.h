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
// 유니티의 tag 문자열을 ObjectTag로 변환한다.
ObjectTag GetTagByString(const std::string& tag);

// 유니티의 Layer 번호[num]를 ObjectLayer로 변환한다.
ObjectLayer GetLayerByNum(int num);

// ObjectTag에 따른 ObjectType을 반환한다.
ObjectType GetObjectType(ObjectTag tag);
#pragma endregion


#pragma region Class
class Component {
protected:
	Object* mObject{};	// 이 Component를 소유하는 객체

private:
	bool mIsActive   = true;

public:
	Component(Object* object) { mObject = object; }
	virtual ~Component() = default;

	bool IsActive() const { return mIsActive; }
	
	void SetActive(bool isActive) { mIsActive = isActive; }

public:
	// 최초 한 번 호출된다.
	virtual void Awake() {}

	// 객체 활성화 시 호출된다.
	virtual void OnEnable() {}

	// 객체 비활성화 시 호출된다.
	virtual void OnDisable() {}

	// Update 호출 전 한 번 호출된다.
	virtual void Start() {}

	// 매 프레임 호출된다.
	virtual void Update() {}

	// 매 프레임 호출된다.
	virtual void Animate() {}

	// 객체가 소멸될 시 호출된다.
	virtual void OnDestroy() {}

	// 동적 리소스를 해제한다.
	virtual void Release() {}

	virtual void ReleaseUploadBuffers() {}

	// 객체(other)와 충돌 시 호출된다.
	virtual void OnCollisionStay(Object& other) {}
};





class Object : public Transform {
private:
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

	bool IsActive() const				{ return mIsEnable; }
#pragma endregion

#pragma region Setter
	void SetTag(ObjectTag tag);
	void SetName(const std::string& name)	{ mName = name; }
	void SetLayer(ObjectLayer layer)		{ mLayer = layer; }
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