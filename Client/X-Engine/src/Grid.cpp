#include "EnginePch.h"
#include "Grid.h"

#include "Scene.h"
#include "Object.h"
#include "Component/Collider.h"

int Grid::mTileRows = 0;
int Grid::mTileCols = 0;


Grid::Grid(int index, int width, const BoundingBox& bb)
	:
	mIndex(index),
	mBB(bb)
{
	mTileRows = static_cast<int>(width / mkTileHeight);
	mTileCols = static_cast<int>(width / mkTileWidth);
	mTiles = std::vector<std::vector<Tile>>(mTileCols, std::vector<Tile>(mTileRows, Tile::None));
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

	switch (object->GetType()) {
	case ObjectType::DynamicMove:
		mDynamicObjets.insert(object);
		break;
	case ObjectType::Env:
		mEnvObjects.insert(object);
		break;
	default:
		mStaticObjects.insert(object);
		UpdateTiles(Tile::Static, object);
		break;
	}
}

void Grid::UpdateTiles(Tile tile, GridObject* object)
{
	// 정적 오브젝트가 Building 태그인 경우에만 벽으로 설정
	if (object->GetTag() != ObjectTag::Building)
		return;


	// 오브젝트의 타일 기준 인덱스 계산
	Vec3 pos = object->GetPosition();
	Pos index = Scene::I->GetTileUniqueIndexFromPos(pos);
	Scene::I->SetTileFromUniqueIndex(index, tile);

	// 오브젝트의 충돌 박스
	const auto& collider = object->GetCollider();
	if (!collider)
		return;

	// BFS를 통해 주변 타일도 업데이트
	std::queue<Pos> q;
	std::map<Pos, bool> visited;
	q.push(index);

	// q가 빌 때까지 BFS를 돌며 현재 타일이 오브젝트와 충돌 했다면 해당 타일을 업데이트
	while (!q.empty()) {
		Pos curNode = q.front();
		q.pop();

		if (visited[curNode])
			continue;

		visited[curNode] = true;

		for (int dir = 0; dir < 8; ++dir) {
			Pos nextPosT = curNode + gkFront[dir];
			Vec3 nextPosW = Scene::I->GetTilePosFromUniqueIndex(nextPosT);
			nextPosW.y = pos.y;

			BoundingBox bb{nextPosW, Vec3{mkTileWidth, mkTileWidth, mkTileHeight} };

			if (collider->Intersects(bb)) {
				Scene::I->SetTileFromUniqueIndex(nextPosT, tile);
				q.push(nextPosT);
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

	switch (object->GetType()) {
	case ObjectType::DynamicMove:
		mDynamicObjets.erase(object);
		break;
	case ObjectType::Env:
		mEnvObjects.erase(object);
		break;
	default:
		mStaticObjects.erase(object);
		UpdateTiles(Tile::None, object);
		break;
	}
}

bool Grid::Intersects(GridObject* object)
{
	const auto& objCollider = object->GetCollider();
	if (!objCollider) {
		return true;
	}
	return mBB.Intersects(objCollider->GetBS());
}

void Grid::CheckCollisions()
{
	if (!mDynamicObjets.empty()) {
		CheckCollisionObjects(mDynamicObjets);						// 동적 객체간 충돌 검사를 수행한다.
		if (!mStaticObjects.empty()) {
			CheckCollisionObjects(mDynamicObjets, mStaticObjects);	// 동적<->정적 객체간 충돌 검사를 수행한다.
		}
	}
}

float Grid::CheckCollisionsRay(const Ray& ray) const
{
	float minDist = 999.f;
	for (const auto& object : mStaticObjects) {
		if (object->GetTag() != ObjectTag::Building)
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

void Grid::CheckCollisions(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type) const
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






// check collision for each object in objects
void Grid::CheckCollisionObjects(std::unordered_set<GridObject*> objects)
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

			Grid::ProcessCollision(objectA, objectB);
		}
	}
}


// check collision for (each objectsA) <-> (each objectsB)
void Grid::CheckCollisionObjects(std::unordered_set<GridObject*> objectsA, std::unordered_set<GridObject*> objectsB)
{
	for (auto a = objectsA.begin(); a != objectsA.end(); ++a) {
		GridObject* objectA = *a;

		for (auto b = objectsB.begin(); b != objectsB.end(); ++b) {
			GridObject* objectB = *b;

			Grid::ProcessCollision(objectA, objectB);
		}
	}
}

// call collision function if collide
void Grid::ProcessCollision(GridObject* objectA, GridObject* objectB)
{
	if (ObjectCollider::Intersects(*objectA, *objectB)) {
		objectA->OnCollisionStay(*objectB);
		objectB->OnCollisionStay(*objectA);
	}
}