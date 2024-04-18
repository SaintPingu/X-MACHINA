#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class MasterModel;
class Texture;
class ObjectPool;
class ObjectCollider;
#pragma endregion





#pragma region Class
// base class for all entities in Scene
class Animator;
class GameObject : public Object {
	using base = Object;
	using Transform::ReturnToPrevTransform;

private:
	bool mIsSkinMesh = false;

	sptr<const MasterModel>		  mMasterModel{};		// ������ ��
	std::vector<const Transform*> mMergedTransform{};	// ��� ������ transfom (���� ������ ���� ĳ��)
	sptr<Animator>	  mAnimator{};

public:
	GameObject() = default;
	virtual ~GameObject() = default;

	bool IsTransparent() const										{ return GetLayer() == ObjectLayer::Transparent; }
	bool IsSkinMesh() const											{ return mIsSkinMesh; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }
	// �ֻ���(��ǥ) �ؽ��ĸ� ��ȯ�Ѵ�.
	rsptr<Texture> GetTexture() const;

	void SetModel(rsptr<const MasterModel> model);
	void SetModel(const std::string& modelName);

public:
	virtual void Animate() override;
	virtual void Render();
	sptr<Animator> GetAnimator() const { return mAnimator; }

private:
	// ��ü�� ��ġ(pos)�� ���鿡 ���δ�.
	void AttachToGround();
};





// ���������Ͽ� �������ϱ� ���� ��ü
class GridObject : public GameObject {
	using base = GameObject;

private:
	int						mCurrGridIndex = -1;	// ���� ��ġ(position)�� Grid index
	std::unordered_set<int> mGridIndices{};			// ���� Grid indices (�浹��)
	ObjectCollider*			mCollider{};			// my ObjectCollider component (for chaching)

	bool mIsDrawBounding = false;					// collision bounds�� �׸��°�?

public:
	GridObject();
	virtual ~GridObject() = default;

	int GetGridIndex() const								{ return mCurrGridIndex; }
	ObjectCollider* GetCollider() const						{ return mCollider; }
	const std::unordered_set<int>& GetGridIndices() const	{ return mGridIndices; }

	// Grid�� [index]��ġ�� �� ��ü �߽�(position)�� ��ġ�Ѵٴ� ������ �����Ѵ�.
	void SetGridIndex(int index) { mCurrGridIndex = index; mGridIndices.insert(index); }
	// Grid�� [indices]��ġ�鿡 �� ��ü�� ��ġ�Ѵٴ� ������ �����Ѵ�.
	void SetGridIndices(const std::unordered_set<int>& indices) { mGridIndices = indices; }

public:
	virtual void Update() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnDestroy() override;

	// render collision bounds
	virtual void RenderBounds();

	// Scene::UpdateObjectGrid(this)
	virtual void UpdateGrid();

	// Collider�� ���� �� ���ٸ�, �� �Լ��� ���� �����ϵ��� �Ѵ�.
	void RemoveCollider();

	// Collider�� �ٽ� �߰��� �����Ѵ�.
	void ResetCollider();

	// ���� ��ġ�� grid indices�� ��� �����Ѵ�.
	void ClearGridIndices() { mGridIndices.clear(); }
	// collision bounds ������ ���
	void ToggleDrawBoundings() { mIsDrawBounding = !mIsDrawBounding; }
};





// instanced GameObject
class InstObject : public GridObject {
	using base = GridObject;

	using GridObject::Render;

private:
	const int mPoolID{};				// my ID(index) of ObjectPool
	ObjectPool* const mObjectPool{};	// my ObjectPool

	std::function<void()> mUpdateFunc{};

	bool mIsPushed{ false };			// ������ ����(of ������Ʈ Ǯ)�� �� ��ü�� �־��°�? (�ߺ� ���� ����)

public:
	InstObject(ObjectPool* pool, int id);
	virtual ~InstObject() = default;

	int GetPoolID() { return mPoolID; }

public:
	// �ν��Ͻ� ��ü�� ���߿� �� ���� ������ �ϵ��� �Ѵ�.
	virtual void Render() override { PushRender(); }
	virtual void Update() override { mUpdateFunc(); }
	virtual void OnDestroy() override;

	// [mUpdateFunc]�� ���ε��Ѵ�.
	virtual void SetUpdateFunc();
	// ��ü�� ������ ���ۿ� �߰��� ��, Ǯ�κ��� �Ҵ���� structured buffer�� ���� �����Ѵ�.
	virtual void PushFunc(void* structuredBuffer) const;

private:
	// ������ ���ۿ� �� ��ü�� �߰��Ѵ�.
	void PushRender();
	void Pop() { mIsPushed = false; }

	// ���� ��ü ������Ʈ (update ���� x, ��������)
	void UpdateStatic() { Pop(); }
	// ���� ��ü ������Ʈ (update ���� o)
	void UpdateDynamic() { base::Update(); Pop(); }
};





// instanced bullet GameObject
class InstBulletObject : public InstObject {
	using base = InstObject;

public:
	InstBulletObject(ObjectPool* pool, int id) : InstObject(pool, id) {}
	virtual ~InstBulletObject() = default;

public:
	virtual void PushFunc(void* structuredBuffer) const override;

	virtual void UpdateGrid() override;
};
#pragma endregion