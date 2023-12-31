#include "stdafx.h"
#include "Collider.h"

#include "Mesh.h"


#pragma region BoxCollider
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
void SphereCollider::Update()
{
	mBS.Center = Vector3::Add(mBS.OriginCenter, mObject->GetPosition());
}


void SphereCollider::Render() const
{
	MeshRenderer::Render(mBS);
}
#pragma endregion





#pragma region ObjectCollider
ObjectCollider& ObjectCollider::operator=(const ObjectCollider& other) {
	if (this == &other) {
		return *this;
	}

	mIsCollidable   = other.mIsCollidable;
	mSphereCollider = other.mSphereCollider;

	return *this;
}

void ObjectCollider::Start()
{
	mSphereCollider = mObject->GetComponent<SphereCollider>();
	if (!mSphereCollider) {
		mObject->RemoveComponent<ObjectCollider>();
		return;
	}

	std::vector<const Transform*> mergedTransform{};
	Transform::MergeTransform(mergedTransform, mObject);

	for (auto transform : mergedTransform) {
		const Object* object = transform->GetObj<Object>();
		const auto& components = std::move(object->GetComponents<BoxCollider>());

		for (auto& boxCollider : components) {
			mOBBList.emplace_back(&boxCollider->mBox);				// insert at "All"
			mBoxColliders.emplace_back(boxCollider);
		}
	}

	UpdateCollidable();
	Update();
}
void ObjectCollider::Update()
{
	if (!mIsCollidable) {
		return;
	}

	for (auto& boxCollider : mBoxColliders) {
		boxCollider->Update();
	}

	mSphereCollider->Update();
}


void ObjectCollider::Render() const
{
	//#define RENDER_BOUNDING_SPHERE
	for (auto& boxCollider : mBoxColliders) {
		boxCollider->Render();
	}

#ifdef RENDER_BOUNDING_SPHERE
	if (mSphereCollider) {
		mSphereCollider->Render();
	}
#endif
}


bool ObjectCollider::Intersects(const BoundingSphere& sphere) const
{
	for (auto& box : mOBBList) {
		if (box->Intersects(sphere)) {
			return true;
		}
	}

	return false;
}
bool ObjectCollider::Intersects(const BoundingBox& srcBox) const
{
	for (auto& box : mOBBList) {
		if (box->Intersects(srcBox)) {
			return true;
		}
	}

	return false;
}
bool ObjectCollider::Intersects(const BoundingOrientedBox& srcBox) const
{
	for (auto& box : mOBBList) {
		if (box->Intersects(srcBox)) {
			return true;
		}
	}

	return false;
}
bool ObjectCollider::Intersects(const std::vector<MyBoundingOrientedBox*>& boxes) const
{
	for (auto& srcBox : boxes) {
		if (Intersects(*srcBox)) {
			return true;
		}
	}

	return false;
}
//    My  - Other
// 1. BS <-> BS
// 2. 둘 다 OBB가 없다면	-> true
// 3. 하나만 OBB가 있다면
// 3-1. 내가   있다면	-> OBB <-> BS
// 3-2. 상대가 있다면		-> BS  <-> OBB
// 4. 둘 다 있다면		-> OBB <-> OBB
bool ObjectCollider::Intersects(const Object& other) const
{
	auto otherCollider = other.GetComponent<ObjectCollider>();

	if (!IsCollidable() || !otherCollider->IsCollidable()) {
		return false;
	}

	auto& otherBS = otherCollider->GetBS();
	auto& myBS    = GetBS();
	if (!IntersectsBS(otherBS)) {
		return false;
	}

	auto& myOBBList    = mOBBList;
	auto& otherOBBList = otherCollider->mOBBList;

	size_t myOBBSize    = myOBBList.size();
	size_t otherOBBSize = otherOBBList.size();

	// Only one object has or does not has a bounding box
	bool a = otherOBBSize <= 0;
	bool b = myOBBSize <= 0;
	if (a || b) {
		if (a && b) {
			return true;	// both don't have a bounding box
		}

		if (a) {			// Only I have a bounding box
			for (auto& box : myOBBList) {
				if (box->Intersects(otherBS)) {
					return true;
				}
			}
		}
		else if (b) {		// Only the other has a bounding box
			if (myBS.IntersectBoxes(otherOBBList)) {
				return true;
			}
		}

		return false;
	}

	// both have a bounding box
	if (Intersects(otherOBBList)) {
		return true;
	}

	return false;
}


void ObjectCollider::UpdateCollidable()
{
	mIsCollidable = false;
	if (mSphereCollider || mOBBList.size() > 0) {
		mIsCollidable = true;
	}
}
#pragma endregion
