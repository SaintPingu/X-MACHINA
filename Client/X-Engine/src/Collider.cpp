#include "EnginePch.h"
#include "Component/Collider.h"

#include "MeshRenderer.h"
#include "Object.h"

#pragma region BoxCollider
BoxCollider& BoxCollider::operator=(const BoxCollider& other)
{
	if (this == &other) {
		return *this;
	}

	mBox = other.mBox;

	return *this;
}

void BoxCollider::Update()
{
	mBox.Transform(mObject->GetWorldTransform());
}


void BoxCollider::Render() const
{
	MeshRenderer::Render(mBox);
}
bool BoxCollider::Intersects(rsptr<Collider> other)
{
	switch (other->GetType()) {
	case Type::Box:
		return mBox.Intersects(reinterpret_cast<BoxCollider*>(other.get())->mBox);
	case Type::Sphere:
		return mBox.Intersects(reinterpret_cast<SphereCollider*>(other.get())->mBS);
	default:
		assert(0);
		break;
	}

	return false;
}
bool BoxCollider::Intersects(const BoundingBox& bb)
{
	return mBox.Intersects(bb);
}
bool BoxCollider::Intersects(const BoundingSphere& bs)
{
	return mBox.Intersects(bs);
}
bool BoxCollider::Intersects(const Ray& ray, float& dist)
{
	return mBox.Intersects(_VECTOR(ray.Position), _VECTOR(ray.Direction), dist);
}
#pragma endregion





#pragma region SphereCollider
SphereCollider& SphereCollider::operator=(const SphereCollider& other)
{
	if (this == &other) {
		return *this;
	}

	mBS = other.mBS;

	return *this;
}

void SphereCollider::Update()
{
	mBS.Transform(mObject->GetWorldTransform());
}


void SphereCollider::Render() const
{
	MeshRenderer::Render(mBS);
}
bool SphereCollider::Intersects(rsptr<Collider> other)
{
	switch (other->GetType()) {
	case Type::Box:
		return mBS.Intersects(reinterpret_cast<BoxCollider*>(other.get())->mBox);
	case Type::Sphere:
		return mBS.Intersects(reinterpret_cast<SphereCollider*>(other.get())->mBS);
	default:
		assert(0);
		break;
	}

	return false;
}
bool SphereCollider::Intersects(const BoundingBox& bb)
{
	return mBS.Intersects(bb);
}
bool SphereCollider::Intersects(const BoundingSphere& bs)
{
	return mBS.Intersects(bs);
}
bool SphereCollider::Intersects(const Ray& ray, float& dist)
{
	return mBS.Intersects(_VECTOR(ray.Position), _VECTOR(ray.Direction), dist);
}
#pragma endregion





#pragma region ObjectCollider
void ObjectCollider::Awake()
{
	base::Awake();

	// SphereCollider�� ��������, ������ ObjectCollider�� �����Ѵ�.
	mSphereCollider = mObject->GetComponent<SphereCollider>();
	if (!mSphereCollider) {
		static_cast<GridObject*>(mObject)->RemoveCollider();
		return;
	}

	// ��ü�� ��� BoxCollider�� bounding box���� �����´�.
	const auto& mergedTransform = mObject->GetObj<GameObject>()->GetMergedTransform();

	for (auto transform : mergedTransform) {
		const Object* object = transform->GetObj<Object>();
		const auto& colliders = object->GetComponents<Collider>();

		for (auto& collider : colliders) {
			if (collider != mSphereCollider) {
				mColliders.push_back(collider);
			}
		}
	}

	Update();
}

// ��� Collider�� transform ������ update�Ѵ�.
void ObjectCollider::Update()
{
	for (auto& collider : mColliders) {
		collider->Update();
	}

	mSphereCollider->Update();
}


void ObjectCollider::Render() const
{
	if (!IsActive()) {
		return;
	}

	for (auto& collider : mColliders) {
		collider->Render();
	}

	#define RENDER_BOUNDING_SPHERE
#ifdef RENDER_BOUNDING_SPHERE
	if (mSphereCollider) {
		mSphereCollider->Render();
	}
#endif
}

bool ObjectCollider::Intersects(const ObjectCollider* other) const
{
	const auto& aBS = GetBS();
	const auto& bBS = other->GetBS();

	// �� ��ü�� Bounding Sphere �켱 �˻�
	if (!aBS.Intersects(bBS)) {
		return false;
	}

	const auto& aColliders = mColliders;
	const auto& bColliders = other->GetColliders();

	bool aHasCollider = !aColliders.empty();
	bool bHasCollider = !bColliders.empty();
	if (!aHasCollider && !bHasCollider) {
		return true;
	}

	if (aHasCollider) {
		for (const auto& a : aColliders) {
			if (a->Intersects(bBS)) {
				return true;
			}
		}
	}
	else if (bHasCollider) {
		for (const auto& b : bColliders) {
			if (b->Intersects(aBS)) {
				return true;
			}
		}
	}
	else {
		for (const auto& a : aColliders) {
			for (const auto& b : bColliders) {
				if (a->Intersects(b)) {
					return true;
				}
			}
		}
	}

	return false;
}

bool ObjectCollider::Intersects(const BoundingBox& bb) const
{
	for (auto& collider : mColliders) {
		if (collider->Intersects(bb)) {
			return true;
		}
	}

	return false;
}

bool ObjectCollider::Intersects(const BoundingSphere& bs) const
{
	for (auto& collider : mColliders) {
		if (collider->Intersects(bs)) {
			return true;
		}
	}

	return false;
}

// �� ObjectCollider �浹ó�� �˰���
//         [A]   <->    [B]
// 1.      BS    <->    BS
// 2.   Collider <-> Collider
bool ObjectCollider::Intersects(const GridObject& a, const GridObject& b)
{
	const auto& colliderA = a.GetCollider();
	const auto& colliderB = b.GetCollider();

	// �ݵ�� �� ��ü ��� ObjectCollider�� ������ �־�� �Ѵ�.
	if (!colliderA || !colliderB) {
		return false;
	}

	return colliderA->Intersects(colliderB);
}
#pragma endregion