#pragma once
#include "Transform.h"

// defines
#define COMPONENT_ABSTRACT( parent, className )							\
private:																\
	using base = parent;												\
																		\
public:																	\
	className(Object* object) : parent(object) {}						\
	virtual ~className() = default;										\

#define COMPONENT( parent, className )									\
private:																\
	using base = parent;												\
																		\
public:																	\
    static const int mID = static_cast<int>(ComponentID::className);	\
    className(Object* object) : parent(object) { }						\
	virtual int GetID() const override { return mID; }					\


// Class Declarations
class Object;

// Enum Classes
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
// constexpr size_t NUM_COMPONENT = static_cast<size_t>(ComponentID::_count);

enum class ObjectTag : DWORD {
	Unspecified		= 0x000,
	Player			= 0x001,
	Building		= 0x002,
	ExplosiveSmall	= 0x004,
	ExplosiveBig	= 0x008,
	Tank			= 0x010,
	Helicopter		= 0x020,
	Background		= 0x040,
	Bullet			= 0x080,
	Billboard		= 0x100,
	Terrain			= 0x200,
	Water			= 0x400,
	Sprite			= 0x800,
};

enum class ObjectLayer {
	Default = 0,
	Transparent,
	Water
};

enum class ObjectType {
	Static = 0,
	Environment,	// static, no collision
	Dynamic,
	DynamicMove,
};

ObjectTag GetTagByName(const std::string& name);
ObjectLayer GetLayerByNum(int num);
ObjectType GetObjectType(ObjectTag tag);






class Component {
public:
	Object* mObject{};

public:
	Component(Object* object) { mObject = object; }
	virtual ~Component() = default;

	virtual int GetID() const { return -1; }

	virtual void Update() {}
	virtual void Start() {}

	virtual void ReleaseUploadBuffers() {}

	virtual void OnCollisionStay(Object& other) {}
};






class Object : public Transform {
private:
	std::vector<sptr<Component>> mComponents{};
	std::unordered_set<const Object*> mCollisionObjects{};

protected:
	std::string mName{};
	ObjectTag mTag{};
	ObjectLayer mLayer{};
	ObjectType mType{};

private:
	void StartComponents();
	void UpdateComponents();

	sptr<Component> GetCopyComponent(rsptr<Component> component);

public:
	Object() : Transform(this) { }
	virtual ~Object() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Getter ] /////

	ObjectTag GetTag() const { return mTag; }
	ObjectLayer GetLayer() const { return mLayer; }
	ObjectType GetType() const { return mType; }

	const std::string& GetName() const { return mName; }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Setter ] /////

	void SetTag(ObjectTag tag);
	void SetLayer(ObjectLayer layer) { mLayer = layer; }

	void SetName(const std::string& name) { mName = name; }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Component ] /////
	template<class T>
	sptr<T> GetComponent() const {
		sptr<T> result{};

		for (auto component : mComponents) {
			if (component->GetID() == T::mID) {
				result = std::static_pointer_cast<T>(component);
				break;
			}
		}

		return result;
	}

	template<class T>
	std::vector<sptr<T>> GetComponents() const {
		std::vector<sptr<T>> result{};

		for (auto& component : mComponents) {
			if (component->GetID() == T::mID) {
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
		for (auto& it = mComponents.begin(); it != mComponents.end(); ++it) {
			component = *it;
			if (component->GetID() == T::mID) {
				mComponents.erase(it);
				return;
			}
		}
	}


	void CopyComponents(const Object& src);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Others ] /////
	void ReleaseUploadBuffers();

	void ProcessComponents(std::function<void(sptr<Component>)> processFunc);

	virtual void Start();
	virtual void Update() override;

	void OnCollisionStay(Object& other);
};