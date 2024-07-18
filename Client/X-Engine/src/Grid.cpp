#include "EnginePch.h"
#include "Grid.h"

#include "Scene.h"
#include "Object.h"
#include "Component/Collider.h"

int Grid::mTileRows = 0;
int Grid::mTileCols = 0;


inline bool IsNotBuilding(ObjectTag tag)
{
	return tag != ObjectTag::Building && tag != ObjectTag::DissolveBuilding;
}

void CollisionManager::AddCollisionPair(ObjectTag tagA, ObjectTag tagB)
{
	mPairs.insert({ tagA, tagB });

	if (!mObjects.contains(tagA)) {
		mObjects[tagA].rehash(10);
	}
	if (!mObjects.contains(tagB)) {
		mObjects[tagB].rehash(10);
	}
}

bool CollisionManager::AddObject(GridObject* object)
{
	const ObjectTag tag = object->GetTag();

	if (mObjects[tag].contains(object)) {
		return false;
	}

	mObjects[tag].insert(object);

	switch (object->GetType())
	{
	case ObjectType::DynamicMove:
		mDynamicObjets.insert(object);
		break;
	case ObjectType::Static:
		mStaticObjects.insert(object);
		break;
	default:
		break;
	}

	return true;
}

bool CollisionManager::RemoveObject(GridObject* object)
{
	const ObjectTag tag = object->GetTag();

	if (!mObjects[tag].contains(object)) {
		return false;
	}

	mObjects[tag].erase(object);

	switch (object->GetType())
	{
	case ObjectType::DynamicMove:
		mDynamicObjets.erase(object);
		break;
	case ObjectType::Static:
		mStaticObjects.erase(object);
		break;
	default:
		break;
	}

	return true;
}

bool CollisionManager::IsContainTagInPairs(ObjectTag tag)
{
	if (mObjects.contains(tag)) {
		return true;
	}

	return false;
}

void CollisionManager::UpdateTag(GridObject* object, ObjectTag beforeTag)
{
	if (mObjects[beforeTag].contains(object)) {
		mObjects[beforeTag].erase(object);
	}

	AddObject(object);
}

void CollisionManager::CheckCollisions()
{
	for (Pair pair : mPairs) {
		auto& groupA = mObjects[pair.first];
		auto& groupB = mObjects[pair.second];

		if (groupA.empty() || groupB.empty())
			continue;

		if ((DWORD)pair.first == (DWORD)pair.second)
			CheckCollisionObjects(groupA);
		else 
			CheckCollisionObjects(groupA, groupB);
	}
}

float CollisionManager::CheckCollisionsRay(const Ray& ray) const
{
	float minDist = 999.f;
	for (const auto& object : mStaticObjects) {
		// 정적 오브젝트가 Building 태그인 경우에만 벽으로 설정
		if (IsNotBuilding(object->GetTag()))
			continue;

		float dist = 100;
		for (const auto& collider : object->GetCollider()->GetColliders()) {
			if (collider->Intersects(ray, dist)) {
				minDist = min(minDist, dist);
			}
		}
	}

	return minDist;
}

void CollisionManager::CheckCollisions(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type) const
{
	if (type & CollisionType::Dynamic) {
		for (const auto& object : mDynamicObjets) {
			if (object->GetCollider()->Intersects(collider)) {
				out.push_back(object);
			}
		}
	}
	if (type & CollisionType::Static) {
		for (const auto& object : mStaticObjects) {
			if (object->GetCollider()->Intersects(collider)) {
				out.push_back(object);
			}
		}
	}
}

void CollisionManager::CheckCollisionObjects(std::unordered_set<GridObject*> objects)
{
	for (auto a = objects.begin(); a != std::prev(objects.end()); ++a) {
		GridObject* objectA = *a;
		if (!objectA->IsActive()) {
			continue;
		}

		for (auto b = std::next(a); b != objects.end(); ++b) {
			GridObject* objectB = *b;
			if (!objectB->IsActive()) {
				continue;
			}

			CollisionManager::ProcessCollision(objectA, objectB);
		}
	}
}

void CollisionManager::CheckCollisionObjects(std::unordered_set<GridObject*> objectsA, std::unordered_set<GridObject*> objectsB)
{
	for (auto a = objectsA.begin(); a != objectsA.end(); ++a) {
		GridObject* objectA = *a;

		for (auto b = objectsB.begin(); b != objectsB.end(); ++b) {
			GridObject* objectB = *b;

			CollisionManager::ProcessCollision(objectA, objectB);
		}
	}
}

void CollisionManager::ProcessCollision(GridObject* objectA, GridObject* objectB)
{
	if (ObjectCollider::Intersects(*objectA, *objectB)) {
		if (!objectA->IsCollided(objectB)) {
			objectA->OnCollisionEnter(*objectB);
			objectB->OnCollisionEnter(*objectA);
		}
	}
	else if (objectA->IsCollided(objectB)) {
		objectA->OnCollisionExit(*objectB);
		objectB->OnCollisionExit(*objectA);
	}
}


Grid::Grid(int index, int width, int height, const BoundingBox& bb)
	:
	mIndex(index),
	mBB(bb)
{
	mTileRows = static_cast<int>(height / mkTileHeight);
	mTileCols = static_cast<int>(width / mkTileWidth);
	mTiles = std::vector<std::vector<Tile>>(mTileRows, std::vector<Tile>(mTileCols, Tile::None));

	mCollisionMgr.AddCollisionPair(ObjectTag::Bullet, ObjectTag::Building);
	mCollisionMgr.AddCollisionPair(ObjectTag::Bullet, ObjectTag::DissolveBuilding);
	mCollisionMgr.AddCollisionPair(ObjectTag::Bullet, ObjectTag::Enemy);
	mCollisionMgr.AddCollisionPair(ObjectTag::Player, ObjectTag::Building);
	mCollisionMgr.AddCollisionPair(ObjectTag::Player, ObjectTag::DissolveBuilding);
	mCollisionMgr.AddCollisionPair(ObjectTag::Player, ObjectTag::Crate);
	mCollisionMgr.AddCollisionPair(ObjectTag::Player, ObjectTag::Item);
}

Tile Grid::GetTileFromUniqueIndex(const Pos& tPos) const
{
	return mTiles[tPos.Z][tPos.X];
}

void Grid::SetTileFromUniqueIndex(const Pos& tPos, Tile tile)
{
	mTiles[tPos.Z][tPos.X] = tile;
}

void Grid::AddObject(GridObject* object)
{
	if (mObjects.count(object)) {
		return;
	}

	mObjects.insert(object);

	if (ObjectType::Static == object->GetType()) {
		UpdateTiles(Tile::Static, object);
	}

	if (!mCollisionMgr.IsContainTagInPairs(object->GetTag())) {
		return;
	}

	mCollisionMgr.AddObject(object);
}

void Grid::UpdateTiles(Tile tile, GridObject* object)
{
	// 정적 오브젝트가 Building 태그인 경우에만 벽으로 설정
	if (IsNotBuilding(object->GetTag()))
		return;

	// BFS를 통해 주변 타일도 업데이트
	std::queue<Pos> q;
	std::map<Pos, bool> visited;

	// 오브젝트의 충돌 박스
	for (const auto& collider : object->GetComponent<ObjectCollider>()->GetColliders()) {
		if (collider->GetType() != Collider::Type::Box) {
			continue;
		}

		// 오브젝트의 타일 기준 인덱스 계산
		Vec3 pos = collider->GetCenter();
		Pos index = Scene::I->GetTileUniqueIndexFromPos(pos);
		Scene::I->SetTileFromUniqueIndex(index, tile);
		q.push(index);

		// q가 빌 때까지 BFS를 돌며 현재 타일이 오브젝트와 충돌 했다면 해당 타일을 업데이트
		while (!q.empty()) {
			Pos curNode = q.front();
			q.pop();

			if (visited[curNode])
				continue;

			visited[curNode] = true;

			for (int dir = 0; dir < 4; ++dir) {
				Pos nextPosT = curNode + gkFront[dir];
				if (nextPosT.X < 0 || nextPosT.Z < 0) {
					continue;
				}

				Vec3 nextPosW = Scene::I->GetTilePosFromUniqueIndex(nextPosT);
				nextPosW.y = pos.y;

				BoundingBox bb{ nextPosW, Vec3{mkTileWidth, mkTileWidth, mkTileHeight} };

				if (collider->Intersects(bb)) {
					Scene::I->SetTileFromUniqueIndex(nextPosT, tile);
					q.push(nextPosT);
				}
			}
		}
	}
}

void Grid::RemoveObject(GridObject* object)
{
	if (!mObjects.count(object)) {
		return;
	}

	mObjects.erase(object);

	if (ObjectType::Static == object->GetType()) {
		UpdateTiles(Tile::None, object);
	}

	if (!mCollisionMgr.IsContainTagInPairs(object->GetTag())) {
		return;
	}

	mCollisionMgr.RemoveObject(object);
}

bool Grid::Intersects(GridObject* object)
{
	const auto& objCollider = object->GetCollider();
	if (!objCollider) {
		return true;
	}
	return mBB.Intersects(objCollider->GetBS());
}

void Grid::UpdateTag(GridObject* object, ObjectTag beforeTag)
{
	if (!mObjects.count(object)) {
		return;
	}

	mCollisionMgr.UpdateTag(object, beforeTag);
}

void Grid::CheckCollisions()
{
	mCollisionMgr.CheckCollisions();
}

float Grid::CheckCollisionsRay(const Ray& ray) const
{
	return mCollisionMgr.CheckCollisionsRay(ray);
}

void Grid::CheckCollisions(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type) const
{
	mCollisionMgr.CheckCollisions(collider, out, type);
}
