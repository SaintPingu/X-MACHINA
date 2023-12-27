#include "stdafx.h"
#include "Component.h"
#include "Collider.h"

static DWORD dynamiObjects {
	static_cast<DWORD>(ObjectTag::Player) |
	static_cast<DWORD>(ObjectTag::Tank) |
	static_cast<DWORD>(ObjectTag::Helicopter) |
	static_cast<DWORD>(ObjectTag::Bullet) |
	static_cast<DWORD>(ObjectTag::ExplosiveBig) |
	static_cast<DWORD>(ObjectTag::ExplosiveSmall)
};

static DWORD dynamicMoveObjects {
	static_cast<DWORD>(ObjectTag::Player) |
	static_cast<DWORD>(ObjectTag::Tank) |
	static_cast<DWORD>(ObjectTag::Helicopter) |
	static_cast<DWORD>(ObjectTag::Bullet)
};

static DWORD environmentObjects{
	static_cast<DWORD>(ObjectTag::Unspecified) |
	static_cast<DWORD>(ObjectTag::Background) |
	static_cast<DWORD>(ObjectTag::Billboard) |
	static_cast<DWORD>(ObjectTag::Terrain)
};


ObjectTag GetTagByName(const std::string& name)
{
	if (name == "Building") {
		return ObjectTag::Building;
	}
	else if (name == "Explosive_small" || name == "Explosive_static") {
		return ObjectTag::ExplosiveSmall;
	}
	else if (name == "Explosive_big") {
		return ObjectTag::ExplosiveBig;
	}
	else if (name == "Tank") {
		return ObjectTag::Tank;
	}
	else if (name == "Helicopter") {
		return ObjectTag::Helicopter;
	}
	else if (name == "Background") {
		return ObjectTag::Background;
	}
	else if (name == "Billboard") {
		return ObjectTag::Billboard;
	}
	else if (name == "Sprite") {
		return ObjectTag::Sprite;
	}
	else if (name != "Untagged") {
		//assert(0);
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
	if (static_cast<DWORD>(tag) & dynamicMoveObjects) {
		return ObjectType::DynamicMove;
	}
	else if (static_cast<DWORD>(tag) & dynamiObjects) {
		return ObjectType::Dynamic;
	}
	else if (static_cast<DWORD>(tag) & environmentObjects) {
		return ObjectType::Environment;
	}

	return ObjectType::Static;
}








////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Component ] /////
template<class T>
void CopyComponent(rsptr<Component> src, sptr<Component>& dst)
{
	sptr<T>& my = static_pointer_cast<T>(dst);
	rsptr<T> other = static_pointer_cast<T>(src);
	*my = *other;
}

sptr<Component> Object::GetCopyComponent(rsptr<Component> component)
{
	sptr<Component> result{};

	switch (component->GetID()) {
	case BoxCollider::mID:
		result = std::make_shared<BoxCollider>(this);
		CopyComponent<BoxCollider>(component, result);
		break;
	case SphereCollider::mID:
		result = std::make_shared<SphereCollider>(this);
		CopyComponent<SphereCollider>(component, result);
		break;
	case ObjectCollider::mID:
		result = std::make_shared<ObjectCollider>(this);
		CopyComponent<ObjectCollider>(component, result);
		break;
	default:
		assert(0);
		break;
	}

	if (result) {
		result->mObject = this;
	}

	return result;
}
void Object::CopyComponents(const Object& src)
{
	const auto& components = src.GetAllComponents();
	for (auto& component : components) {
		sptr<Component> copy = GetCopyComponent(component);
		mComponents.emplace_back(copy);
	}
}










////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

void Object::ProcessComponents(std::function<void(sptr<Component>)> processFunc) {
	for (auto& component : mComponents) {
		if (component) {
			processFunc(component);
		}
	}
}

void Object::StartComponents()
{
	ProcessComponents([](sptr<Component> component) {
		component->Start();
		});
}

void Object::UpdateComponents()
{
	ProcessComponents([](sptr<Component> component) {
		component->Update();
		});
}

void Object::SetTag(ObjectTag tag)
{
	mTag = tag;
	mType = GetObjectType(tag);
}

void Object::Start()
{
	Transform::Update();
	StartComponents();
}

void Object::Update()
{
	mCollisionObjects.clear();
	Transform::Update();
	UpdateComponents();
}

void Object::OnCollisionStay(Object& other)
{
	if (mCollisionObjects.count(&other)) {
		return;
	}
	mCollisionObjects.insert(&other);

	ProcessComponents([&other](sptr<Component> component) {
		component->OnCollisionStay(other);
		});
}

void Object::ReleaseUploadBuffers()
{
	ProcessComponents([](sptr<Component> component) {
		component->ReleaseUploadBuffers();
		});
}