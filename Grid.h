#pragma once

#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
class Grid {
private:
	int mIndex{};

	BoundingBox mBB{};

	std::unordered_set<GameObject*> mObjects{};
	std::unordered_set<GameObject*> mEnvObjects{};
	std::unordered_set<GameObject*> mStaticObjects{};
	std::unordered_set<GameObject*> mDynamicObjets{};

public:
	Grid()          = default;
	virtual ~Grid() = default;

	const BoundingBox& GetBB() const { return mBB; }
	const auto& GetObjects() const { return mObjects; }

public:
	bool Empty() const { return mObjects.empty(); }

	void Init(int index, const BoundingBox& bb);

	void AddObject(GameObject* object);
	void RemoveObject(GameObject* object);

	void CheckCollisions();
};
#pragma endregion