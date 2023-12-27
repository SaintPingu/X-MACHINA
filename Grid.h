#pragma once
class CGameObject;

class CGrid {
private:
	int mIndex{};
	BoundingBox mBB{};
	std::unordered_set<CGameObject*> mObjects{};
	std::unordered_set<CGameObject*> mEnvObjects{};
	std::unordered_set<CGameObject*> mStaticObjects{};
	std::unordered_set<CGameObject*> mDynamicObjects{};

public:
	CGrid();
	~CGrid();

	void Init(int index, const BoundingBox& bb);
	const BoundingBox& GetBB() const { return mBB; }
	void RemoveObject(CGameObject* object);
	void AddObject(CGameObject* object);

	const std::unordered_set<CGameObject*>& GetObjects() const { return mObjects; }
	bool Empty() const { return mObjects.empty(); }

	void CheckCollisions();
};