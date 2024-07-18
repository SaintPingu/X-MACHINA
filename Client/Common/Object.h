#pragma once

#pragma region Include
#include "Component/Component.h"
#include "UploadBuffer.h"
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
	friend class Scene;
	friend class RenderedAbility;
	friend class DynamicEnvironmentMappingManager;

private:
	bool mIsSkinMesh = false;
	bool mUseShadow = true;

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
	void SetUseShadow(bool useShadow) { mUseShadow = useShadow; }

public:
	sptr<Animator> GetAnimator() const { return mAnimator; }
	bool GetUseShadow() const { return mUseShadow; }

protected:
	virtual void Animate() override;
	virtual void Render();

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

	// render collision bounds
	virtual void RenderBounds();

	// Scene::UpdateObjectGrid(this)
	virtual void UpdateGrid();

	// Collider�� �ٽ� �߰��� �����Ѵ�.
	void ResetCollider();

	// ���� ��ġ�� grid indices�� ��� �����Ѵ�.
	void ClearGridIndices() { mGridIndices.clear(); }

public:
	virtual void SetTag(ObjectTag tag) override;

protected:
	virtual void Awake() override;
	virtual void Update() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnDestroy() override;
};





// instanced GameObject
class InstObject : public GridObject {
	using base = GridObject;

	using GridObject::Render;

protected:
	const int mPoolID{};				// my ID(index) of ObjectPool
	ObjectPool* const mObjectPool{};	// my ObjectPool

	std::function<void()> mUpdateFunc{};

public:
	InstObject(ObjectPool* pool, int id);
	virtual ~InstObject() = default;

	int GetPoolID() { return mPoolID; }

public:
	// [mUpdateFunc]�� ���ε��Ѵ�.
	virtual void SetUpdateFunc();
	// ��ü�� ������ ���ۿ� �߰��� ��, Ǯ�κ��� �Ҵ���� structured buffer�� ���� �����Ѵ�.
	virtual void PushFunc(int buffIdx, UploadBuffer<InstanceData>* buffer) const;

	void Return();

protected:
	// �ν��Ͻ� ��ü�� ���߿� �� ���� ������ �ϵ��� �Ѵ�.
	virtual void Render() override { PushRender(); }
	virtual void Update() override { mUpdateFunc(); }

private:
	// ������ ���ۿ� �� ��ü�� �߰��Ѵ�.
	void PushRender();

	// ���� ��ü ������Ʈ (update ���� x)
	void UpdateStatic() { }
	// ���� ��ü ������Ʈ (update ���� o)
	void UpdateDynamic() { base::Update(); }
};





// instanced bullet GameObject
class InstBulletObject : public InstObject {
	using base = InstObject;

public:
	InstBulletObject(ObjectPool* pool, int id) : InstObject(pool, id) {}
	virtual ~InstBulletObject() = default;

public:
	virtual void UpdateGrid() override;
};





// use dynamic environment mapping Object
class DynamicEnvironmentMappingManager : public Singleton<DynamicEnvironmentMappingManager> {
	friend Singleton;

private:
	static constexpr UINT mkMaxMRTCount = 1;

	std::array<sptr<class MultipleRenderTarget>, mkMaxMRTCount> mMRTs{};
	std::array<std::array<sptr<class CameraObject>, 6>, mkMaxMRTCount> mCameras{};

	std::map<Object*, class MultipleRenderTarget*> mDynamicEnvironmentObjectMap;

public:
	DynamicEnvironmentMappingManager() = default;
	virtual ~DynamicEnvironmentMappingManager() = default;

public:
	UINT AddObject(Object* object);
	void RemoveObject(Object* object);

public:
	void Init();
	void UpdatePassCB(sptr<class Camera> camera, UINT index);
	void Render(const std::set<GridObject*>& objects);

private:
	void BuildCubeFaceCamera();
};

#pragma endregion