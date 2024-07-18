#pragma once

#pragma region ClassForwardDecl
class GridObject;
class Collider;
#pragma endregion


#pragma region enum
enum class Tile: UINT8{
	None = 0,
	Static,
	Dynamic,
};


#pragma region Using
using namespace Path;
#pragma endregion

class CollisionManager {
private:
	using Pair = std::pair<ObjectTag, ObjectTag>;

	std::map<ObjectTag, std::unordered_set<GridObject*>> mObjects;
	std::set<Pair> mPairs;

	std::unordered_set<GridObject*> mStaticObjects{};
	std::unordered_set<GridObject*> mDynamicObjets{};

public:
	const auto& GetDynamicObjects() const { return mDynamicObjets; }
	const auto& GetObjectsFromTag(ObjectTag tag) { return mObjects[tag]; }

public:
	void AddCollisionPair(ObjectTag tagA, ObjectTag tagB);
	bool AddObject(GridObject* object);
	bool RemoveObject(GridObject* object);
	bool IsContainTagInPairs(ObjectTag tag);

	void UpdateTag(GridObject* object, ObjectTag beforeTag);

public:
	void CheckCollisions();
	float CheckCollisionsRay(const Ray& ray) const;
	void CheckCollisions(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type = CollisionType::All) const;

	static void CheckCollisionObjects(std::unordered_set<GridObject*> objects);
	static void CheckCollisionObjects(std::unordered_set<GridObject*> objectsA, std::unordered_set<GridObject*> objectsB);
	static void ProcessCollision(GridObject* objectA, GridObject* objectB);
};


class Grid {
private:
	const int mIndex{};
	const BoundingBox mBB{};
	std::vector<std::vector<Tile>> mTiles{};

	std::unordered_set<GridObject*> mObjects{};	
	CollisionManager mCollisionMgr{};


public:
	static constexpr float mkTileHeight = 0.5f;
	static constexpr float mkTileWidth = 0.5f;
	static int mTileRows;
	static int mTileCols;

public:
	Grid(int index, int width, int height, const BoundingBox& bb);
	virtual ~Grid() = default;

	int GetIndex() const { return mIndex; }
	const BoundingBox& GetBB() const { return mBB; }

	// return all objects
	const auto& GetObjects() const { return mObjects; }
	const auto& GetDynamicObjects() const { return mCollisionMgr.GetDynamicObjects(); }
	const auto& GetObjectsFromTag(ObjectTag tag) { return mCollisionMgr.GetObjectsFromTag(tag); }

	Tile GetTileFromUniqueIndex(const Pos& tPos) const;
	void SetTileFromUniqueIndex(const Pos& tPos, Tile tile);

public:
	bool Empty() const { return mObjects.empty(); }

	// add [object] to gird
	void AddObject(GridObject* object);

	// remove [object] from gird
	void RemoveObject(GridObject* object);

	void UpdateTag(GridObject* object, ObjectTag beforeTag);

	bool Intersects(GridObject* object);

	// BFS를 활용하여 타일 업데이트
	void UpdateTiles(Tile tile, GridObject* object);

	// collision check for objects contained in grid
	void CheckCollisions();
	float CheckCollisionsRay(const Ray& ray) const;
	void CheckCollisions(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type = CollisionType::All) const;
};
#pragma endregion