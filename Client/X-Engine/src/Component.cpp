#include "EnginePch.h"
#include "Component/Component.h"
#include "Component/Collider.h"



UINT32 Object::sID = 0;

namespace {

	constexpr DWORD gkDynamicObjects {
		ObjectTag::Unspecified		|
		ObjectTag::Player			|
		ObjectTag::Bullet			|
		ObjectTag::Enemy			|
		ObjectTag::Dynamic
	};

	constexpr DWORD gkDynamicMoveObjects {
		ObjectTag::Unspecified		|
		ObjectTag::Player			|
		ObjectTag::Bullet			|
		ObjectTag::Enemy			|
		ObjectTag::Dynamic
	};

	constexpr DWORD gkEnvObjects {
		ObjectTag::Unspecified		|
		ObjectTag::Environment		|
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

#pragma region Functions
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






#pragma region Component
void Component::SetActive(bool isActive)
{
	if (isActive)
	{
		if (mIsActive) {
			return;
		}

		OnEnable();
	}
	else {
		if (!mIsActive) {
			return;
		}

		OnDisable();
	}
}

void Component::FirstUpdate()
{
	if (!mIsAwake) {
		Awake();
	}
	if (!mIsActive) {
		return;
	}

	SetActive(true);
	if (!mIsStart) {
		Start();
	}
	Update();
	UpdateFunc = std::bind(&Component::Update, this);
}
#pragma endregion








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

Transform* Object::FindFrame(const std::string& frameName)
{
	if (GetName() == frameName) {
		return this;
	}

	Transform* transform{};
	if (mSibling) {
		if (transform = mSibling->GetObj<Object>()->FindFrame(frameName)) {
			return transform;
		}
	}
	if (mChild) {
		if (transform = mChild->GetObj<Object>()->FindFrame(frameName)) {
			return transform;
		}
	}

	return nullptr;
}


void Object::SetActive(bool isActive)
{
	if (isActive) {
		if (IsActive()) {
			return;
		}

		OnEnable();
	}
	else {
		if (!IsActive()) {
			return;
		}

		OnDisable();
	}
}


void Object::Awake()
{
	assert(!mIsAwake);
	mIsAwake = true;

	Transform::Awake();
	ProcessComponents([](rsptr<Component> component) {
		if (!component->IsAwake()) {
			component->Awake();
		}
		});
}

void Object::OnEnable()
{
	if (!mIsAwake) {
		Awake();
	}

	assert(!mIsEnable);
	mIsEnable = true;

	ProcessComponents([](rsptr<Component> component) {
			component->SetActive(true);
		});
	Transform::ComputeWorldTransform();

	if (!mIsStart) {
		Start();
	}
}

void Object::OnDisable()
{
	assert(mIsEnable);
	mIsEnable = false;

	ProcessComponents([](rsptr<Component> component) {
		component->SetActive(false);
		});
}

void Object::Start()
{
	if (!mIsAwake) {
		Awake();
	}
	if (!mIsEnable) {
		OnEnable();
	}

	assert(!mIsStart);
	mIsStart = true;

	ProcessComponents([](rsptr<Component> component) {
		if (component->IsActive() && !component->IsStart()) {
			component->Start();
		}
		});
	Transform::ComputeWorldTransform();
}

void Object::Update()
{
	Transform::BeforeUpdateTransform();
	mCollisionObjects.clear();
	ProcessComponents([](rsptr<Component> component) {
		if (component->IsActive()) {
			component->UpdateFunc();
		}
		});
	Transform::ComputeWorldTransform();
}

void Object::Animate()
{
	ProcessComponents([](rsptr<Component> component) {
		if (component->IsActive()) {
			component->Animate();
		}
		});
	Transform::ComputeWorldTransform();
}

void Object::LateUpdate()
{
	ProcessComponents([](rsptr<Component> component) {
		if (component->IsActive()) {
			component->LateUpdate();
		}
		});
	Transform::ComputeWorldTransform();
}

void Object::OnDestroy()
{
	assert(!mIsDestroyed);
	mIsDestroyed = true;

	SetActive(false);
	ProcessComponents([](rsptr<Component> component) {
		component->OnDestroy();
		});
	Transform::OnDestroy();
}

void Object::Release()
{
	ProcessComponents([](rsptr<Component> component) {
		component->Release();
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
	std::vector<sptr<Component>> components = mComponents;
	for (auto& component : components) {
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
	const auto& id = typeid(*component);
	if (id == typeid(BoxCollider)) {
		result = CopyComponent<BoxCollider>(component, this);
	}
	else if (id == typeid(SphereCollider)) {
		result = CopyComponent<SphereCollider>(component, this);
	}

	return result;
}
#pragma endregion