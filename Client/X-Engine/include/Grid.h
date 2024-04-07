#pragma once

#pragma region ClassForwardDecl
class GridObject;
#pragma endregion


#pragma region Class
enum class Tile: UINT8{
	None = 0,
	Static,
	Dynamic,
};

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

public:
	bool Empty() const { return mObjects.empty(); }

	// set grid's index and bounding box
	void Init(int index, int width, const BoundingBox& bb);

	// add [object] to gird
	void AddObject(GridObject* object);

	void AddObjectInTiles(Tile objectType, GridObject* object);

	// remove [object] from gird
	void RemoveObject(GridObject* object);

	// collision check for objects contained in grid
	void CheckCollisions();
};
#pragma endregion