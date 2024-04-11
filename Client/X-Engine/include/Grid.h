#pragma once

#pragma region ClassForwardDecl
class GridObject;
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
	int mIndex{};
	std::vector<std::vector<Tile>> mTiles{};

	BoundingBox mBB{};

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
	Grid()          = default;
	virtual ~Grid() = default;

	const BoundingBox& GetBB() const	{ return mBB; }

	// return all objects
	const auto& GetObjects() const		{ return mObjects; }

	Tile GetTileFromUniqueIndex(const Pos& tPos) const;
	void SetTileFromUniqueIndex(const Pos& tPos, Tile tile);

public:
	bool Empty() const { return mObjects.empty(); }

	// set grid's index and bounding box
	void Init(int index, int width, const BoundingBox& bb);

	// add [object] to gird
	void AddObject(GridObject* object);

	// remove [object] from gird
	void RemoveObject(GridObject* object);

	// BFS를 활용하여 타일 업데이트
	void UpdateTiles(Tile tile, GridObject* object);

	// collision check for objects contained in grid
	void CheckCollisions();
	float CheckCollisionsRay(const Ray& ray) const;

};
#pragma endregion