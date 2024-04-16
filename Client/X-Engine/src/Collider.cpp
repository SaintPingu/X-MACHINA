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
		const auto& components = object->GetComponents<BoxCollider>();

		for (auto& boxCollider : components) {
			mOBBList.emplace_back(&boxCollider->mBox);
			mBoxColliders.emplace_back(boxCollider);
		}
	}

	Update();
}

// ��� Collider�� transform ������ update�Ѵ�.
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

// �� ObjectCollider �浹ó�� �˰���
//       [A] <-> [B]
// 1.    BS  <-> BS
// 2-1.  OBB <-> OBB : �� �� OBB�� �ִٸ�
// 2-2.  OBB <-> BS  : A�� �ִٸ�
// 2-3.  BS  <-> OBB : B�� �ִٸ�
// 3.       true     : �� �� ���ٸ�
bool ObjectCollider::Intersects(const GridObject& a, const GridObject& b)
{
	const auto& colliderA = a.GetCollider();
	const auto& colliderB = b.GetCollider();

	// �ݵ�� �� ��ü ��� ObjectCollider�� ������ �־�� �Ѵ�.
	if (!colliderA || !colliderB) {
		return false;
	}

	const auto& bsA = colliderA->GetBS();
	const auto& bsB = colliderB->GetBS();

	// �� ��ü�� Bounding Sphere �켱 �˻�
	if (!bsA.Intersects(bsB)) {
		return false;
	}

	const auto& obbListA = colliderA->GetOBBList();
	const auto& obbListB = colliderB->GetOBBList();

	const bool aHasOBB = !obbListA.empty();
	const bool bHasOBB = !obbListB.empty();

	// �� ��ü�� OBB�� ������ �ִٸ�
	if (aHasOBB || bHasOBB) {
		if (aHasOBB && bHasOBB) {	 // �� ��ü ��� OBB�� ������ �ִٸ� OBB<->OBB �� �浹�˻�
			return ObjectCollider::Intersects(obbListA, obbListB);
		}
		// �ϳ��� ��ü�� OBB�� ������ �ִٸ� OBB<->BS �浹�˻�
		else if (aHasOBB) {
			return ObjectCollider::Intersects(obbListA, bsB);
		}
		else {
			return ObjectCollider::Intersects(obbListB, bsA);
		}
	}

	// �� ��ü ��� OBB�� ���ٸ� �浹(true)
	return true;
}
#pragma endregion