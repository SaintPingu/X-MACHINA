#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class MasterModel;
class Texture;
class ObjectPool;
class ObjectCollider;
#pragma endregion





#pragma region Class
// base class for all entities in Scene
class GameObject : public Object {
	using base = Object;
	using Transform::ReturnToPrevTransform;

private:
	sptr<const MasterModel> mMasterModel{};

	std::vector<const Transform*> mMergedTransform{};	// ��� ������ transfom (���� ������ ���� ĳ��)


public:
#pragma region C/Dtor
	GameObject() = default;
	virtual ~GameObject() = default;
#pragma endregion

#pragma region Getter
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }

	// �ֻ���(��ǥ) �ؽ��ĸ� ��ȯ�Ѵ�.
	rsptr<Texture> GetTexture() const;
#pragma endregion

#pragma region Setter
	bool IsTransparent() const { return mLayer == ObjectLayer::Transparent; }

	void SetModel(rsptr<const MasterModel> model);
#pragma endregion

public:
	virtual void Render();

	virtual void Update();

	virtual void Enable();
	virtual void Disable();

	// [frameName]�� Transform�� ���� �������� ã�� ��ȯ�Ѵ� (������ nullptr)
	Transform* FindFrame(const std::string& frameName);

private:
	// ��ü�� ��ġ(pos)�� ���鿡 ���δ�.
	void AttachToGround();
};





class GridObject : public GameObject {
	using base = GameObject;

private:
	std::unordered_set<int> mGridIndices{};				// ���� Grid indices (�浹��)
	int mCurrGridIndex = -1;							// ���� ��ġ�� Grid index
	sptr<ObjectCollider> mCollider{};

	bool mIsDrawBounding = false;	// collision bounds�� �׸��°�?

public:
	GridObject();
	virtual ~GridObject() = default;

	int GetGridIndex() const { return mCurrGridIndex; }
	rsptr<ObjectCollider> GetCollider() const { return mCollider; }
	const std::unordered_set<int>& GetGridIndices() const { return mGridIndices; }

	void SetGridIndex(int index) { mCurrGridIndex = index; }
	void SetGridIndices(const std::unordered_set<int>& indices) { mGridIndices = indices; }

public:
	virtual void Enable() override;
	virtual void Disable() override;

	// render collision bounds
	virtual void RenderBounds();

	void ClearGridIndices() { mGridIndices.clear(); }
	void ToggleDrawBoundings() { mIsDrawBounding = !mIsDrawBounding; }
};





// instanced GameObject
// ���� ������ �ʴ´�.
class InstObject : public GridObject {
private:
	int mPoolID{};
	using base = GridObject;

	using GridObject::Render;

private:
	ObjectPool* mBuffer{};

	std::function<void()> mUpdateFunc{};

	bool mIsPushed{ false };	// buffer�� �� ��ü�� �־��°�?

public:
	InstObject()          = default;
	virtual ~InstObject() = default;

	int GetPoolID() { return mPoolID; }

	void SetBuffer(ObjectPool* buffer, int id);

public:
	virtual void Render() override { Push(); }
	virtual void Update() override { mUpdateFunc(); }
	virtual void OnDestroy() override;

private:
	// �ν��Ͻ� ���ۿ� �� ��ü�� �߰��Ѵ�.
	void Push();
	void Pop() { mIsPushed = false; }

	// ���� ��ü ������Ʈ (update ���� x, ��������)
	void UpdateStatic();
	// ���� ��ü ������Ʈ (update ���� o)
	void UpdateDynamic();
};
#pragma endregion