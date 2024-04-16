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
#pragma endregion





#pragma region ObjectCollider
void ObjectCollider::Awake()
{
	base::Awake();

	// SphereCollider를 가져오고, 없으면 ObjectCollider를 제거한다.
	mSphereCollider = mObject->GetComponent<SphereCollider>();
	if (!mSphereCollider) {
		static_cast<GridObject*>(mObject)->RemoveCollider();
		return;
	}

	// 객체의 모든 BoxCollider와 bounding box들을 가져온다.
	const auto& mergedTransform = mObject->GetObj<GameObject>()->GetMergedTransform();

	for (auto transform : mergedTransform) {
		const Object* object = transform->GetObj<Object>();
		const auto& components = object->GetComponents<BoxCollider>();

		for (auto& boxCollider : components) {
			mOBBList.emplace_back(&boxCollider->mBox);
			mBoxColliders.emplace_back(boxCollider);
		}
	}

	Update();
}

// 모든 Collider의 transform 정보를 update한다.
void ObjectCollider::Update()
{
	for (auto& boxCollider : mBoxColliders) {
		boxCollider->Update();
	}

	mSphereCollider->Update();
}


void ObjectCollider::Render() const
{
	if (!IsActive()) {
		return;
	}

	for (auto& boxCollider : mBoxColliders) {
		boxCollider->Render();
	}

	#define RENDER_BOUNDING_SPHERE
#ifdef RENDER_BOUNDING_SPHERE
	if (mSphereCollider) {
		mSphereCollider->Render();
	}
#endif
}

// 두 ObjectCollider 충돌처리 알고리즘
//       [A] <-> [B]
// 1.    BS  <-> BS
// 2-1.  OBB <-> OBB : 둘 다 OBB가 있다면
// 2-2.  OBB <-> BS  : A만 있다면
// 2-3.  BS  <-> OBB : B만 있다면
// 3.       true     : 둘 다 없다면
bool ObjectCollider::Intersects(const GridObject& a, const GridObject& b)
{
	const auto& colliderA = a.GetCollider();
	const auto& colliderB = b.GetCollider();

	// 반드시 두 객체 모두 ObjectCollider를 가지고 있어야 한다.
	if (!colliderA || !colliderB) {
		return false;
	}

	const auto& bsA = colliderA->GetBS();
	const auto& bsB = colliderB->GetBS();

	// 두 객체간 Bounding Sphere 우선 검사
	if (!bsA.Intersects(bsB)) {
		return false;
	}

	const auto& obbListA = colliderA->GetOBBList();
	const auto& obbListB = colliderB->GetOBBList();

	const bool aHasOBB = !obbListA.empty();
	const bool bHasOBB = !obbListB.empty();

	// 한 객체라도 OBB를 가지고 있다면
	if (aHasOBB || bHasOBB) {
		if (aHasOBB && bHasOBB) {	 // 두 객체 모두 OBB를 가지고 있다면 OBB<->OBB 간 충돌검사
			return ObjectCollider::Intersects(obbListA, obbListB);
		}
		// 하나의 객체만 OBB를 가지고 있다면 OBB<->BS 충돌검사
		else if (aHasOBB) {
			return ObjectCollider::Intersects(obbListA, bsB);
		}
		else {
			return ObjectCollider::Intersects(obbListB, bsA);
		}
	}

	// 두 객체 모두 OBB가 없다면 충돌(true)
	return true;
}
#pragma endregion