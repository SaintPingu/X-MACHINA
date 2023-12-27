#include "stdafx.h"
#include "Collider.h"
#include "DXGIMgr.h"

#include "Mesh.h"
#include "Camera.h"


// [ BoxCollider ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BoxCollider::Update()
{
	mBox.Transform(mObject->GetWorldTransform());
}

void BoxCollider::Render() const
{
	MeshRenderer::Render(mBox);
}

// [ SphereCollider ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SphereCollider::Update()
{
	mBS.Center = Vector3::Add(mBS.OriginCenter, mObject->GetPosition());
}

void SphereCollider::Render() const
{
	MeshRenderer::Render(mBS);
}




// [ ObjectCollider ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
		auto components = object->GetComponents<BoxCollider>();
		for(auto& boxCollider : components) {
			auto& box = boxCollider->mBox;
			mOBBList.emplace_back(&box);				// insert at "All"
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


void ObjectCollider::UpdateCollidable()
{
	mIsCollidable = false;
	if (mSphereCollider || mOBBList.size() > 0) {
		mIsCollidable = true;
	}
}


//////////////////* Intersects *//////////////////
bool ObjectCollider::IntersectsBS(const BoundingSphere& sphere) const
{
	return mSphereCollider->mBS.Intersects(sphere);
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

bool ObjectCollider::Intersects(const std::vector<BoundingOrientedBox>& boxes) const
{
	for (auto& srcBox : boxes) {
		if (Intersects(srcBox)) {
			return true;
		}
	}

	return false;
}

bool ObjectCollider::IsInFrustum(const BoundingFrustum& frustum) const
{
	return frustum.Intersects(mSphereCollider->mBS);
}

//    My  - Other
// 1. BS <-> BS
// 2. 둘 다 OBB가 없다면	-> true
// 3. 하나만 OBB가 있다면
// 3-1. 내가   없다면	-> BS <-> OBB
// 3-2. 상대가 없다면		-> OBB <-> BS
// 4. 둘 다 있다면		-> OBB <-> OBB
bool ObjectCollider::Intersects(const Object& other) const
{
	auto otherCollider = other.GetComponent<ObjectCollider>();

	if (!IsCollidable() || !otherCollider->IsCollidable()) {
		return false;
	}

	auto& otherBS = otherCollider->GetBS();
	auto& bs = GetBS();
	if (!bs.Intersects(otherBS)) {
		return false;
	}

	auto& myOBBList = mOBBList;
	auto& otherOBBList = otherCollider->mOBBList;

	size_t myOBBSize = myOBBList.size();
	size_t otherOBBSize = otherOBBList.size();

	// Only one object has or does not has a bounding box
	bool a = myOBBSize <= 0;
	bool b = otherOBBSize <= 0;
	if (a || b) {
		if (a && b) {
			return true;	// have no bounding box
		}

		if (a) {			// Only I have a bounding box
			if (bs.IntersectBoxes(otherOBBList)) {
				return true;
			}
		}
		else if (b) {		// Only the other has a bounding box
			for (auto& box : myOBBList) {
				if (box->Intersects(otherBS)) {
					return true;
				}
			}
		}

		return false;
	}

	// both have a bounding box
	for (auto& myBox : myOBBList) {
		for (auto& otherBox : otherOBBList) {
			if (myBox->Intersects(*otherBox)) {
				return true;
			}
		}
	}

	return false;
}


#define RENDER_BOUNDING_SPHERE
void ObjectCollider::Render() const
{
	for (auto& boxCollider : mBoxColliders) {
		boxCollider->Render();
	}

#ifdef RENDER_BOUNDING_SPHERE
	if (mSphereCollider) {
		mSphereCollider->Render();
	}
#endif
}