#include "EnginePch.h"
#include "Component/Collider.h"

#include "MeshRenderer.h"
#include "Object.h"






#pragma region Collider
BoxCollider* Collider::GetBoxCollider()
{
	if (GetType() != Type::Box) {
		return nullptr;
	}

	return static_cast<BoxCollider*>(this);
}

SphereCollider* Collider::GetSphereCollider()
{
	if (GetType() != Type::Sphere) {
		return nullptr;
	}

	return static_cast<SphereCollider*>(this);
}

void Collider::OnEnable()
{
	base::OnEnable();

	UpdateTransform();
}
#pragma endregion





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
	UpdateTransform();
}


void BoxCollider::Render(Vec4 color) const
{
	if (IsActive()) {
		MeshRenderer::Render(mBox, color);
	}
}
bool BoxCollider::Intersects(rsptr<Collider> other) const
{
	if (!IsActive()) {
		return false;
	}

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
bool BoxCollider::Intersects(const BoundingBox& bb) const
{
	if (!IsActive()) {
		return false;
	}

	return mBox.Intersects(bb);
}
bool BoxCollider::Intersects(const BoundingOrientedBox& obb) const
{
	if (!IsActive()) {
		return false;
	}

	return mBox.Intersects(obb);;
}
bool BoxCollider::Intersects(const BoundingSphere& bs) const
{
	if (!IsActive()) {
		return false;
	}

	return mBox.Intersects(bs);
}
bool BoxCollider::Intersects(const Ray& ray, float& dist) const
{
	if (!IsActive()) {
		return false;
	}

	if(Vector3::IsZero(ray.Direction)) {
		return false;
	}
	return mBox.Intersects(_VECTOR(ray.Position), XMVector3Normalize(_VECTOR(ray.Direction)), dist);
}
void BoxCollider::UpdateTransform()
{
	mBox.Transform(mObject->GetWorldTransform());
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
	UpdateTransform();
}


void SphereCollider::Render(Vec4 color) const
{
	if (IsActive()) {
		MeshRenderer::Render(mBS, color);
	}
}
bool SphereCollider::Intersects(rsptr<Collider> other) const
{
	if (!IsActive()) {
		return false;
	}

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
bool SphereCollider::Intersects(const BoundingBox& bb) const
{
	if (!IsActive()) {
		return false;
	}

	return mBS.Intersects(bb);
}
bool SphereCollider::Intersects(const BoundingOrientedBox& obb) const
{
	if (!IsActive()) {
		return false;
	}

	return mBS.Intersects(obb);
}
bool SphereCollider::Intersects(const BoundingSphere& bs) const
{
	if (!IsActive()) {
		return false;
	}

	return mBS.Intersects(bs);
}
bool SphereCollider::Intersects(const Ray& ray, float& dist) const
{
	if (!IsActive()) {
		return false;
	}

	return mBS.Intersects(_VECTOR(ray.Position), _VECTOR(ray.Direction), dist);
}
void SphereCollider::UpdateTransform()
{
	mBS.Transform(mObject->GetWorldTransform());
}
#pragma endregion





#pragma region ObjectCollider
void ObjectCollider::Awake()
{
	base::Awake();

	// SphereCollider를 가져오고, 없으면 ObjectCollider를 제거한다.
	mSphereCollider = mObject->GetComponent<SphereCollider>();
	const auto& gridObject = mObject->GetObj<GridObject>();
	if (!mSphereCollider) {
		gridObject->RemoveComponent<ObjectCollider>();
		return;
	}

	// 객체의 모든 BoxCollider와 bounding box들을 가져온다.
	const auto& mergedTransform = gridObject->GetMergedTransform();

	for (auto transform : mergedTransform) {
		const Object* object = transform->GetObj<Object>();
		const auto& colliders = object->GetComponents<Collider>();

		for (auto& collider : colliders) {
			if (collider != mSphereCollider) {
				collider->SetActive(true);
				mColliders.push_back(collider);
			}
		}
	}

	Update();
}

void ObjectCollider::Start()
{
	base::Start();

	for (auto& collider : mColliders) {
		collider->Start();
	}
}


void ObjectCollider::Update()
{
	base::Update();

	for (auto& collider : mColliders) {
		if (collider->IsActive()) {
			collider->Update();
		}
	}
}

void ObjectCollider::Render(Vec4 color) const
{
	if (!IsActive()) {
		return;
	}

	for (auto& collider : mColliders) {
		collider->Render(color);
	}

	//#define RENDER_BOUNDING_SPHERE
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

	// 두 객체간 Bounding Sphere 우선 검사
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

	if (aHasCollider && bHasCollider) {
		for (const auto& a : aColliders) {
			for (const auto& b : bColliders) {
				if (a->Intersects(b)) {
					return true;
				}
			}
		}
	}
	else {
		if (aHasCollider) {
			for (const auto& a : aColliders) {
				if (a->Intersects(bBS)) {
					return true;
				}
			}
		}
		else {
			for (const auto& b : bColliders) {
				if (b->Intersects(aBS)) {
					return true;
				}
			}
		}
	}

	return false;
}


bool ObjectCollider::Intersects(rsptr<Collider> collider) const
{
	switch (collider->GetType()) {
	case Collider::Type::Box:
		return Intersects(collider->GetBoxCollider()->mBox);
	case Collider::Type::Sphere:
		return Intersects(collider->GetSphereCollider()->mBS);
	default:
		assert(0);
		break;
	}
	return false;
}

// 두 ObjectCollider 충돌처리 알고리즘
//         [A]   <->    [B]
// 1.      BS    <->    BS
// 2.   Collider <-> Collider
bool ObjectCollider::Intersects(const GridObject& a, const GridObject& b)
{
	const auto& colliderA = a.GetCollider();
	const auto& colliderB = b.GetCollider();

	// 반드시 두 객체 모두 ObjectCollider를 가지고 있어야 한다.
	if (!colliderA || !colliderB) {
		return false;
	}

	// 둘 중 하나라도 비활성 상태라면 검사하지 않는다.
	if (!colliderA->IsActive() || !colliderB->IsActive()) {
		return false;
	}

	return colliderA->Intersects(colliderB);
}
#pragma endregion