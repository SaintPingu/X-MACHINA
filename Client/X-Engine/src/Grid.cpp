#include "EnginePch.h"
#include "Grid.h"

#include "Scene.h"
#include "Object.h"
#include "Component/Collider.h"


int Grid::mTileRows = 0;
int Grid::mTileCols = 0;

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






Tile Grid::GetTileFromUniqueIndex(const Pos& tPos) const
{
	return mTiles[tPos.Z][tPos.X];
}

void Grid::SetTileFromUniqueIndex(const Pos& tPos, Tile tile)
{
	mTiles[tPos.Z][tPos.X] = tile;
}

void Grid::Init(int index, int width, const BoundingBox& bb)
{
	mIndex = index;
	mBB    = bb;

	mTileRows = static_cast<int>(width / mkTileHeight);
	mTileCols = static_cast<int>(width / mkTileWidth);
	mTiles = std::vector<std::vector<Tile>>(mTileCols, std::vector<Tile>(mTileRows, Tile::None));
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
		AddObjectInTiles(Tile::Static, object);
		break;
	}
}

void Grid::AddObjectInTiles(Tile tile, GridObject* object)
{
	// 정적 오브젝트가 Building 태그인 경우에만 벽으로 설정
	if (object->GetTag() != ObjectTag::Building)
		return;

	// 오브젝트의 타일 기준 인덱스 계산
	Vec3 pos = object->GetPosition().xz();
	Pos index = scene->GetTileUniqueIndexFromPos(pos);

	scene->SetTileFromUniqueIndex(index, tile);

	const auto& collider = object->GetCollider();
	if (!collider)
		return;

	const int kRad = static_cast<int>(object->GetCollider()->GetBS().Radius);

	for (int offsetZ = -kRad; offsetZ <= kRad; ++offsetZ) {
		for (int offsetX = -kRad; offsetX <= kRad; ++offsetX) {
			const int neighborX = index.X + offsetX;
			const int neighborZ = index.Z + offsetZ;
			scene->SetTileFromUniqueIndex(Pos{ neighborZ, neighborX }, tile);
		}
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
		AddObjectInTiles(Tile::None, object);
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

float Grid::CheckCollisionsRay(const Ray& ray) const
{
	for (const auto& object : mStaticObjects) {
		if (object->GetTag() != ObjectTag::Building)
			continue;

		float temp;
		float dist = Vec3::Distance(ray.Position, object->GetPosition());
		if (ray.Intersects(object->GetCollider()->GetBS(), temp))
			return dist;
	}

	return 999.f;
}
