#pragma once

#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
class Grid {
private:
	int mIndex{};

	BoundingBox mBB{};

	std::unordered_set<GameObject*> mObjects{};			// all objects (env, static, dynamic, ...)
	std::unordered_set<GameObject*> mEnvObjects{};
	std::unordered_set<GameObject*> mStaticObjects{};
	std::unordered_set<GameObject*> mDynamicObjets{};

public:
	Grid()          = default;
	virtual ~Grid() = default;

	const BoundingBox& GetBB() const { return mBB; }

	// return all objects
	const auto& GetObjects() const { return mObjects; }

public:
	bool Empty() const { return mObjects.empty(); }

	// set grid's index and bounding box
	void Init(int index, const BoundingBox& bb);

	// add [object] to gird
	void AddObject(GameObject* object);
	// remove [object] from gird
	void RemoveObject(GameObject* object);

	// collision check for objects contained in grid
	void CheckCollisions();
};
#pragma endregion