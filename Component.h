#pragma once

#pragma region Include
#include "Transform.h"
#pragma endregion


#pragma region Define
#define COMPONENT_ABSTRACT( className, parent )							\
private:																\
	using base = parent;												\
																		\
public:																	\
	className(Object* object) : parent(object) {}						\
	virtual ~className() = default;

#define COMPONENT( className, parent )									\
private:																\
	using base = parent;												\
																		\
public:																	\
    static const int ID = static_cast<int>(ComponentID::className);		\
    className(Object* object) : parent(object) { }						\
    virtual ~className() = default; 									\
	virtual int GetID() const override { return ID; }
#pragma endregion


#pragma region ClassForwardDecl
class Object;
#pragma endregion


#pragma region EnumClass
enum class ComponentID {
	BoxCollider = 0,
	SphereCollider,
	ObjectCollider,
	Camera,
	Rigidbody,

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
	_count
};

enum class ObjectTag : DWORD {
	Unspecified = 0x000,
	Player = 0x001,
	Building = 0x002,
	ExplosiveSmall = 0x004,
	ExplosiveBig = 0x008,
	Tank = 0x010,
	Helicopter = 0x020,
	Background = 0x040,
	Bullet = 0x080,
	Billboard = 0x100,
	Terrain = 0x200,
	Water = 0x400,
	Sprite = 0x800,
};

enum class ObjectLayer {
	Default = 0,
	Transparent,
	Water
};

enum class ObjectType {
	Static = 0,
	Environment,	// also static, but no collision.
	Dynamic,
	DynamicMove,
};
#pragma endregion


#pragma region Function
ObjectTag GetTagByName(const std::string& name);
ObjectLayer GetLayerByNum(int num);
ObjectType GetObjectType(ObjectTag tag);
#pragma endregion


#pragma region Class
class Component {
public:
	Object* mObject{};

private:
	static constexpr int kNotID = -1;

public:
	Component(Object* object) { mObject = object; }
	virtual ~Component() = default;

	virtual int GetID() const { return kNotID; }

	virtual void Start() {}
	virtual void Update() {}
	virtual void Release() {}

	virtual void ReleaseUploadBuffers() {}

	virtual void OnCollisionStay(Object& other) {}
};





class Object : public Transform {
protected:
	std::string	mName{};
	ObjectTag	mTag{};
	ObjectLayer mLayer{};
	ObjectType	mType{};

private:
	std::vector<sptr<Component>> mComponents{};
	std::unordered_set<const Object*> mCollisionObjects{};

public:
#pragma region C/Dtor
	Object() : Transform(this) { }
	virtual ~Object() { Release(); }
#pragma endregion

#pragma region Getter
	ObjectTag GetTag() const { return mTag; }
	ObjectLayer GetLayer() const { return mLayer; }
	ObjectType GetType() const { return mType; }

	const std::string& GetName() const { return mName; }
#pragma endregion

#pragma region Setter
	void SetTag(ObjectTag tag);
	void SetLayer(ObjectLayer layer) { mLayer = layer; }

	void SetName(const std::string& name) { mName = name; }
#pragma endregion



#pragma region Component
	template<class T>
	sptr<T> GetComponent() const {
		for (const auto& component : mComponents) {
			if (component->GetID() == T::ID) {
				return std::static_pointer_cast<T>(component);
			}
		}

		return nullptr;
	}

	template<class T>
	std::vector<sptr<T>> GetComponents() const {
		std::vector<sptr<T>> result{};

		for (const auto& component : mComponents) {
			if (component->GetID() == T::ID) {
				result.emplace_back(std::static_pointer_cast<T>(component));
			}
		}

		return result;
	}

	std::vector<sptr<Component>> GetAllComponents() const {
		return mComponents;
	}

	template<class T>
	sptr<T> AddComponent() {
		mComponents.emplace_back(std::make_shared<T>((Object*)this));
		return std::static_pointer_cast<T>(mComponents.back());
	}

	template<class T>
	void RemoveComponent() {
		sptr<Component> component{};
		for (auto it = mComponents.begin(); it != mComponents.end(); ++it) {
			component = *it;
			if (component->GetID() == T::ID) {
				mComponents.erase(it);
				return;
			}
		}
	}

	void CopyComponents(const Object& src);
#pragma endregion

	virtual void Start();
	virtual void Update() override;
	virtual void Release();

	virtual void ReleaseUploadBuffers();

	void OnCollisionStay(Object& other);

private:
	void ProcessComponents(std::function<void(sptr<Component>)> processFunc);
	void StartComponents();
	void UpdateComponents();
	void ReleaseComponents();

	sptr<Component> GetCopyComponent(rsptr<Component> component);
};
#pragma endregion