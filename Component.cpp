#include "stdafx.h"
#include "Component.h"
#include "Collider.h"



namespace {
	constexpr DWORD gkDynamicObjects {
		ObjectTag::Player			|
		ObjectTag::Tank				|
		ObjectTag::Helicopter		|
		ObjectTag::Bullet			|
		ObjectTag::ExplosiveBig		|
		ObjectTag::ExplosiveSmall
	};

	constexpr DWORD gkDynamicMoveObjects {
		ObjectTag::Player			|
		ObjectTag::Tank				|
		ObjectTag::Helicopter		|
		ObjectTag::Bullet
	};

	constexpr DWORD gkEnvObjects {
		ObjectTag::Unspecified		|
		ObjectTag::Environment		|
		ObjectTag::Billboard		|
		ObjectTag::Terrain
	};

	constexpr bool IsDynamicMoveObject(ObjectTag tag)
	{
		return gkDynamicMoveObjects & tag;
	}

	constexpr bool IsDynamicObject(ObjectTag tag)
	{
		return gkDynamicMoveObjects & tag;
	}

	constexpr bool IsEnvObject(ObjectTag tag)
	{
		return gkEnvObjects & tag;
	}
}



#pragma region Object
void Object::SetTag(ObjectTag tag)
{
	mTag  = tag;
	mType = GetObjectType(tag);
}

void Object::CopyComponents(const Object& src)
{
	const auto& components = src.GetAllComponents();
	for (auto& component : components) {
		mComponents.emplace_back(GetCopyComponent(component));
	}
}

void Object::Awake()
{
	mIsAwake = true;
	ProcessComponents([](rsptr<Component> component) {
		component->Awake();
		});
}

void Object::OnEnable()
{
	mIsActive = true;

	Transform::Update();

	ProcessComponents([](rsptr<Component> component) {
		component->OnEnable();
		});
}

void Object::OnDisable()
{
	mIsActive = false;

	ProcessComponents([](rsptr<Component> component) {
		component->OnDisable();
		});
}

void Object::Start()
{
	mIsStart = true;
	ProcessComponents([](rsptr<Component> component) {
		component->Start();
		});
}

void Object::Update()
{
	Transform::Update();
	
	mCollisionObjects.clear();
	ProcessComponents([](rsptr<Component> component) {
		if (component->IsActive()) {
			component->Update();
		}
		});
}

void Object::OnDestroy()
{
	ProcessComponents([](rsptr<Component> component) {
		component->OnDestroy();
		});
}

void Object::Release()
{
	ProcessComponents([](rsptr<Component> component) {
		component->Release();
		});
}

void Object::ReleaseUploadBuffers()
{
	ProcessComponents([](rsptr<Component> component) {
		component->ReleaseUploadBuffers();
		});
}

void Object::OnCollisionStay(Object& other)
{
	// 한 프레임 내에 중복 호출된 경우 무시한다.
	if (mCollisionObjects.count(&other)) {
		return;
	}

	mCollisionObjects.insert(&other);
	ProcessComponents([&other](sptr<Component> component) {
		component->OnCollisionStay(other);
		});
}



void Object::ProcessComponents(std::function<void(rsptr<Component>)> processFunc) {
	for (auto& component : mComponents) {
		if (component) {
			processFunc(component);
		}
	}
}


namespace {
	template<class T>
	inline sptr<T> CopyComponent(rsptr<Component> src, Object* object)
	{
		sptr<T> result = std::make_shared<T>(object);
		rsptr<T> other = static_pointer_cast<T>(src);
		*result        = *other;						// 복사 생성자를 각 Component내에서 구현해야 한다.
		return result;
	}
}

sptr<Component> Object::GetCopyComponent(rsptr<Component> component)
{
	sptr<Component> result{};

	// 복사 생성자가 오버로딩된 컴포넌트에 한해 컴포넌트 복사를 수행한다.
	switch (component->GetID()) {
	case BoxCollider::ID:
		result = CopyComponent<BoxCollider>(component, this);
		break;
	case SphereCollider::ID:
		result = CopyComponent<SphereCollider>(component, this);
		break;
	default:
		assert(0);
		break;
	}

	return result;
}
#pragma endregion


#pragma region Functions
ObjectTag GetTagByString(const std::string& tag)
{
	switch (Hash(tag)) {
	case Hash("Building"):
		return ObjectTag::Building;

	case Hash("Explosive_small"):
	case Hash("Explosive_static"):
		return ObjectTag::ExplosiveSmall;

	case Hash("Explosive_big"):
		return ObjectTag::ExplosiveBig;

	case Hash("Tank"):
		return ObjectTag::Tank;

	case Hash("Helicopter"):
		return ObjectTag::Helicopter;

	case Hash("Background"):
		return ObjectTag::Environment;

	case Hash("Billboard"):
		return ObjectTag::Billboard;

	case Hash("Sprite"):
		return ObjectTag::Sprite;

	default:
		//assert(0);
		break;
	}

	return ObjectTag::Unspecified;
}

ObjectLayer GetLayerByNum(int num)
{
	switch (num) {
	case 0:
		return ObjectLayer::Default;

	case 3:
		return ObjectLayer::Transparent;

	case 4:
		return ObjectLayer::Water;

	default:
		assert(0);
		break;
	}

	return ObjectLayer::Default;
}

ObjectType GetObjectType(ObjectTag tag)
{
	if (::IsDynamicMoveObject(tag)) {
		return ObjectType::DynamicMove;
	}
	else if (::IsDynamicObject(tag)) {
		return ObjectType::Dynamic;
	}
	else if (::IsEnvObject(tag)) {
		return ObjectType::Env;
	}

	return ObjectType::Static;
}
#pragma endregion
