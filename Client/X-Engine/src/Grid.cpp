#include "EnginePch.h"
#include "Grid.h"

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






Pos Grid::GetTileIndexFromPos(const Vec3& pos) const
{
	// 오브젝트의 타일 기준 인덱스 계산
	Vec2 index = Vec2{ Vec2{fabs(mStartPoint)} + Vec2{pos.x, pos.z} } - Vec2{ static_cast<float>(mWidth) } * mVec2Index;

	return Pos{ static_cast<int>(index.y), static_cast<int>(index.x) };
}


Vec3 Grid::GetTilePosFromIndex(const Pos& tPos) const
{
	return Vec3{ tPos.X * mkTileWidth + mStartPoint, 0, tPos.Z * mkTileHeight + mStartPoint };
}


TileObjectType Grid::GetTileObjectTypeFromPos(const Vec3& pos) const
{
	Pos tPos = GetTileIndexFromPos(pos);
	return mTiles[tPos.Z][tPos.X].mType;
}


TileObjectType Grid::GetTileObjectTypeFromUniqueIndex(const Pos& tPos) const
{
	return mTiles[tPos.Z][tPos.X].mType;
}


void Grid::Init(int index, int cols, int width, float startPoint, const BoundingBox& bb)
{
	mIndex = index;
	mBB    = bb;
	mWidth = width;
	mVec2Index = { static_cast<float>(index % cols), static_cast<float>(index / cols)};
	mStartPoint = startPoint;

	mTileRows = static_cast<int>(width / mkTileHeight);
	mTileCols = static_cast<int>(width / mkTileWidth);

	mTiles = std::vector<std::vector<Tile>>(mTileCols, std::vector<Tile>(mTileRows));
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
		AddObjectInTiles(TileObjectType::Static, object);
		break;
	}
}
#include "Scene.h"
void Grid::AddObjectInTiles(TileObjectType objectType, GridObject* object)
{
	// 정적 오브젝트가 Building 태그인 경우에만 벽으로 설정
	if (object->GetTag() != ObjectTag::Building)
		return;

	// 오브젝트의 타일 기준 인덱스 계산
	Vec3 pos = object->GetPosition().xz();
	Pos index = scene->GetTileUniqueIndexFromPos(pos);
	const int tileX = index.X % mTileRows;
	const int tileZ = index.Z % mTileCols;

	// 오브젝트의 바운딩 스피어의 크기만큼 해당하는 모든 타일을 업데이트
	if (tileZ < 0 || tileZ >= mTileRows || tileX < 0 || tileZ >= mTileCols)
		return;

	mTiles[tileZ][tileX].mType = objectType;

	const auto& collider = object->GetCollider();
	if (!collider)
		return;

	const int kRad = static_cast<int>(object->GetCollider()->GetBS().Radius);

	for (int offsetZ = -kRad; offsetZ <= kRad; ++offsetZ) {
		for (int offsetX = -kRad; offsetX <= kRad; ++offsetX) {
			const int neighborX = tileX + offsetX;
			const int neighborZ = tileZ + offsetZ;

			if (neighborX >= 0 && neighborX < mTileRows && neighborZ >= 0 && neighborZ < mTileCols)
				mTiles[neighborZ][neighborX].mType = objectType;
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
		AddObjectInTiles(TileObjectType::None, object);
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