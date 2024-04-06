#include "EnginePch.h"
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






Pos Grid::GetTileIndexFromPos(const Vec3& pos) const
{
	// 오브젝트의 타일 기준 인덱스 계산
	Vec2 index = Vec2{ Vec2{fabs(mStartPoint)} + Vec2{pos.x, pos.z} } - Vec2{ static_cast<float>(mWidth) } * mVec2Index;

	return Pos{ static_cast<int>(index.y), static_cast<int>(index.x) };
}


Vec3 Grid::GetTilePosFromIndex(const Pos& tPos) const
{
	Vec2 pos = Vec2{ static_cast<float>(tPos.X), static_cast<float>(tPos.Z) } - Vec2{ fabs(mStartPoint) } + Vec2{ static_cast<float>(mWidth) } * mVec2Index;

	return Vec3{ pos.x, 0, pos.y };
}


TileObjectType Grid::GetTileObjectTypeFromPos(const Vec3& pos) const
{
	Pos tPos = GetTileIndexFromPos(pos);
	return mTiles[tPos.Z][tPos.X].mType;
}


TileObjectType Grid::GetTileObjectTypeFromIndex(const Pos& tPos) const
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

	mNumTileRows = static_cast<int>(width / mTileHeight);
	mNumTileCols = static_cast<int>(width / mTileWidth);

	mTiles = std::vector<std::vector<Tile>>(mNumTileCols, std::vector<Tile>(mNumTileRows));
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

void Grid::AddObjectInTiles(TileObjectType objectType, GridObject* object)
{
	// 정적 오브젝트가 Building 태그인 경우에만 벽으로 설정
	if (object->GetTag() != ObjectTag::Building)
		return;

	// 오브젝트의 타일 기준 인덱스 계산
	Vec3 pos = object->GetPosition().xz();
	Pos tPos = GetTileIndexFromPos(pos);

	int tPosZ = static_cast<int>(tPos.Z);
	int tPosX = static_cast<int>(tPos.X);

	// 오브젝트의 바운딩 스피어의 크기만큼 해당하는 모든 타일을 업데이트
	if (tPosZ < 0 || tPosZ >= mNumTileRows || tPosX < 0 || tPosX >= mNumTileCols)
		return;

	mTiles[tPosZ][tPosX].mType = objectType;

	const auto& collider = object->GetCollider();
	if (!collider)
		return;

	const int kRad = static_cast<int>(object->GetCollider()->GetBS().Radius);

	for (int offsetZ = -kRad; offsetZ <= kRad; ++offsetZ) {
		for (int offsetX = -kRad; offsetX <= kRad; ++offsetX) {
			const int neighborX = tPosX + offsetX;
			const int neighborZ = tPosZ + offsetZ;

			if (neighborX >= 0 && neighborX < mNumTileRows && neighborZ >= 0 && neighborZ < mNumTileCols)
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