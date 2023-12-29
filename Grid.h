#pragma once
//-----------------------------[Class Declaration]-----------------------------//
class GameObject;
//-----------------------------------------------------------------------------//

class Grid {
private:
	int mIndex{};
	BoundingBox mBB{};
	std::unordered_set<GameObject*> mObjects{};
	std::unordered_set<GameObject*> mEnvObjects{};
	std::unordered_set<GameObject*> mStatiObjects{};
	std::unordered_set<GameObject*> mDynamicObjets{};

public:
	Grid() = default;
	~Grid() = default;

	void Init(int index, const BoundingBox& bb);
	const BoundingBox& GetBB() const { return mBB; }
	void RemoveObject(GameObject* object);
	void AddObject(GameObject* object);

	const auto& GetObjects() const { return mObjects; }
	bool Empty() const { return mObjects.empty(); }

	void CheckCollisions();
};