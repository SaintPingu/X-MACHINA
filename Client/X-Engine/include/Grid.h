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


class Grid {
private:
	const int mIndex{};
	const BoundingBox mBB{};

	std::vector<std::vector<Tile>> mTiles{};

	std::unordered_set<GridObject*> mObjects{};			// all objects (env, static, dynamic, ...)
	std::unordered_set<GridObject*> mEnvObjects{};
	std::unordered_set<GridObject*> mStaticObjects{};
	std::unordered_set<GridObject*> mDynamicObjets{};

public:
	static constexpr float mkTileHeight = 0.5f;
	static constexpr float mkTileWidth = 0.5f;
	static int mTileRows;
	static int mTileCols;

public:
	Grid(int index, int width, const BoundingBox& bb);
	virtual ~Grid() = default;

	int GetIndex() const { return mIndex; }
	const BoundingBox& GetBB() const	{ return mBB; }

	// return all objects
	const auto& GetObjects() const		{ return mObjects; }

	Tile GetTileFromUniqueIndex(const Pos& tPos) const;
	void SetTileFromUniqueIndex(const Pos& tPos, Tile tile);

public:
	bool Empty() const { return mObjects.empty(); }

	// add [object] to gird
	void AddObject(GridObject* object);

	// remove [object] from gird
	void RemoveObject(GridObject* object);

	bool Intersects(GridObject* object);

	// BFS를 활용하여 타일 업데이트
	void UpdateTiles(Tile tile, GridObject* object);

	// collision check for objects contained in grid
	void CheckCollisions();
	float CheckCollisionsRay(const Ray& ray) const;
	void CheckCollisions(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type = CollisionType::All) const;

private:
	static void CheckCollisionObjects(std::unordered_set<GridObject*> objects);
	static void CheckCollisionObjects(std::unordered_set<GridObject*> objectsA, std::unordered_set<GridObject*> objectsB);
	static void ProcessCollision(GridObject* objectA, GridObject* objectB);
};
#pragma endregion