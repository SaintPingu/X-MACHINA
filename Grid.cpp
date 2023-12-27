#include "stdafx.h"
#include "Grid.h"
#include "Object.h"
#include "Collider.h"

// [ CGrid ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CGrid::CGrid()
{
}
CGrid::~CGrid()
{

}

void CGrid::Init(int index, const BoundingBox& bb)
{
	mIndex = index;
	mBB = bb;
}

void CGrid::AddObject(CGameObject* object)
{
	mObjects.insert(object);

	switch (object->GetType()) {
	case ObjectType::DynamicMove:
		mDynamicObjects.insert(object);
		break;
	case ObjectType::Environment:
		mEnvObjects.insert(object);
		break;
	default:
		mStaticObjects.insert(object);
		break;
	}
}
void CGrid::RemoveObject(CGameObject* object)
{
	mObjects.erase(object);
	mEnvObjects.erase(object);
	mDynamicObjects.erase(object);
	mStaticObjects.erase(object);
}

bool ProcessCollision(CGameObject* objectA, CGameObject* objectB)
{
	if (objectA->GetComponent<ObjectCollider>()->Intersects(*objectB)) {
		objectA->OnCollisionStay(*objectB);
		objectB->OnCollisionStay(*objectA);

		return true;
	}

	return false;
}

// check collision for each object in objects
void CheckCollisionObjects(std::unordered_set<CGameObject*> objects)
{
	for (auto& a = objects.begin(); a != std::prev(objects.end()); ++a) {
		auto& objectA = *a;

		for (auto& b = std::next(a); b != objects.end(); ++b) {
			auto& objectB = *b;

			ProcessCollision(objectA, objectB);
		}
	}
}

// check collision for each object in each objects
void CheckCollisionObjects(std::unordered_set<CGameObject*> objects1, std::unordered_set<CGameObject*> objects2)
{
	for (auto& a = objects1.begin(); a != objects1.end(); ++a) {
		auto& objectA = *a;

		for (auto& b = objects2.begin(); b != objects2.end(); ++b) {
			auto& objectB = *b;

			ProcessCollision(objectA, objectB);
		}
	}
}

void CGrid::CheckCollisions()
{
	if (!mDynamicObjects.empty()) {
		::CheckCollisionObjects(mDynamicObjects);
		if (!mStaticObjects.empty()) {
			::CheckCollisionObjects(mDynamicObjects, mStaticObjects);
		}
	}
}