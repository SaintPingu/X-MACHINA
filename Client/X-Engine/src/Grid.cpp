#include "stdafx.h"
#include "Grid.h"

#include "Object.h"
#include "Component/Collider.h"



namespace {
	// call collision function if collide
	void ProcessCollision(GridObject* objectA, GridObject* objectB)
	{
		if (ObjectCollider::Intersects(*objectA, *objectB)) {
			objectA->OnCollisionStay(*objectB);
			objectB->OnCollisionStay(*objectA);
		}
	}

	// check collision for each object in objects
	void CheckCollisionObjects(std::unordered_set<GridObject*> objects)
	{
		for (auto a = objects.begin(); a != std::prev(objects.end()); ++a) {
			GridObject* objectA = *a;

			for (auto b = std::next(a); b != objects.end(); ++b) {
				GridObject* objectB = *b;

				ProcessCollision(objectA, objectB);
			}
		}
	}

	// check collision for (each objectsA) <-> (each objectsB)
	void CheckCollisionObjects(std::unordered_set<GridObject*> objectsA, std::unordered_set<GridObject*> objectsB)
	{
		for (auto a = objectsA.begin(); a != objectsA.end(); ++a) {
			GridObject* objectA = *a;

			for (auto b = objectsB.begin(); b != objectsB.end(); ++b) {
				GridObject* objectB = *b;

				ProcessCollision(objectA, objectB);
			}
		}
	}
}



void Grid::Init(int index, const BoundingBox& bb)
{
	mIndex = index;
	mBB    = bb;
}

void Grid::AddObject(GridObject* object)
{
	if (mObjects.count(object)) {
		return;
	}

	mObjects.insert(object);

	switch (object->GetType()) {
	case ObjectType::DynamicMove:
		mDynamicObjets.insert(object);
		break;
	case ObjectType::Env:
		mEnvObjects.insert(object);
		break;
	default:
		mStaticObjects.insert(object);
		break;
	}
}

void Grid::RemoveObject(GridObject* object)
{
	if (!mObjects.count(object)) {
		return;
	}

	mObjects.erase(object);

	switch (object->GetType()) {
	case ObjectType::DynamicMove:
		mDynamicObjets.erase(object);
		break;
	case ObjectType::Env:
		mEnvObjects.erase(object);
		break;
	default:
		mStaticObjects.erase(object);
		break;
	}
}

void Grid::CheckCollisions()
{
	if (!mDynamicObjets.empty()) {
		::CheckCollisionObjects(mDynamicObjets);						// 동적 객체간 충돌 검사를 수행한다.
		if (!mStaticObjects.empty()) {
			::CheckCollisionObjects(mDynamicObjets, mStaticObjects);	// 동적<->정적 객체간 충돌 검사를 수행한다.
		}
	}
}