#pragma once

#pragma region ClassForwardDecl
class GridObject;
#pragma endregion


#pragma region Class
enum class TileObjectType : UINT8{
	None = 0,
	Static,
	Dynamic,
};

struct Tile {
	TileObjectType mType = TileObjectType::None;
};

class Grid {
private:
	int mIndex{};
	int mWidth{};
	int mCols{};
	float mStartPoint{};
	Vec2 mVec2Index{};

	BoundingBox mBB{};

	std::unordered_set<GridObject*> mObjects{};			// all objects (env, static, dynamic, ...)
	std::unordered_set<GridObject*> mEnvObjects{};
	std::unordered_set<GridObject*> mStaticObjects{};
	std::unordered_set<GridObject*> mDynamicObjets{};

	static constexpr float mTileHeight = 0.5f;
	static constexpr float mTileWidth = 0.5f;
	int mNumTileRows{};
	int mNumTileCols{};

public:
	std::vector<std::vector<Tile>> mTiles{};
	Grid()          = default;
	virtual ~Grid() = default;

	const BoundingBox& GetBB() const	{ return mBB; }

	// return all objects
	const auto& GetObjects() const		{ return mObjects; }

	Pos GetTileIndexFromPos(const Vec3& pos) const;
	Vec3 GetTilePosFromIndex(const Pos& tPos) const;
	TileObjectType GetTileObjectTypeFromPos(const Vec3& pos) const;
	TileObjectType GetTileObjectTypeFromIndex(const Pos& tPos) const;

public:
	bool Empty() const { return mObjects.empty(); }

	// set grid's index and bounding box
	void Init(int index, int cols, int width, float startPoint, const BoundingBox& bb);

	// add [object] to gird
	void AddObject(GridObject* object);

	void AddObjectInTiles(TileObjectType objectType, GridObject* object);

	// remove [object] from gird
	void RemoveObject(GridObject* object);

	// collision check for objects contained in grid
	void CheckCollisions();
};
#pragma endregion