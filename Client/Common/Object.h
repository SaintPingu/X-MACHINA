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

	sptr<const MasterModel>		  mMasterModel{};		// 렌더링 모델
	std::vector<const Transform*> mMergedTransform{};	// 모든 계층의 transfom (빠른 접근을 위한 캐싱)
	sptr<Animator>	  mAnimator{};

public:
	GameObject() = default;
	virtual ~GameObject() = default;

	bool IsTransparent() const										{ return GetLayer() == ObjectLayer::Transparent; }
	bool IsSkinMesh() const											{ return mIsSkinMesh; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }
	// 최상위(대표) 텍스쳐를 반환한다.
	rsptr<Texture> GetTexture() const;

	void SetModel(rsptr<const MasterModel> model);
	void SetModel(const std::string& modelName);

public:
	virtual void Animate() override;
	virtual void Render();
	sptr<Animator> GetAnimator() const { return mAnimator; }

private:
	// 객체의 위치(pos)를 지면에 붙인다.
	void AttachToGround();
};





// 공간분할하여 렌더링하기 위한 객체
class GridObject : public GameObject {
	using base = GameObject;

private:
	int						mCurrGridIndex = -1;	// 현재 위치(position)의 Grid index
	std::unordered_set<int> mGridIndices{};			// 인접 Grid indices (충돌됨)
	ObjectCollider*			mCollider{};			// my ObjectCollider component (for chaching)

	bool mIsDrawBounding = false;					// collision bounds를 그리는가?

public:
	GridObject();
	virtual ~GridObject() = default;

	int GetGridIndex() const								{ return mCurrGridIndex; }
	ObjectCollider* GetCollider() const						{ return mCollider; }
	const std::unordered_set<int>& GetGridIndices() const	{ return mGridIndices; }

	// Grid의 [index]위치에 내 객체 중심(position)이 위치한다는 정보를 저장한다.
	void SetGridIndex(int index) { mCurrGridIndex = index; mGridIndices.insert(index); }
	// Grid의 [indices]위치들에 내 객체가 위치한다는 정보를 저장한다.
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

	// Collider가 사용될 수 없다면, 이 함수를 통해 제거하도록 한다.
	void RemoveCollider();

	// Collider를 다시 추가해 적용한다.
	void ResetCollider();

	// 내가 위치한 grid indices를 모두 제거한다.
	void ClearGridIndices() { mGridIndices.clear(); }
	// collision bounds 렌더링 토글
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

	bool mIsPushed{ false };			// 렌더링 버퍼(of 오브젝트 풀)에 이 객체를 넣었는가? (중복 삽입 방지)

public:
	InstObject(ObjectPool* pool, int id);
	virtual ~InstObject() = default;

	int GetPoolID() { return mPoolID; }

public:
	// 인스턴싱 객체는 나중에 한 번에 렌더링 하도록 한다.
	virtual void Render() override { PushRender(); }
	virtual void Update() override { mUpdateFunc(); }
	virtual void OnDestroy() override;

	// [mUpdateFunc]를 바인딩한다.
	virtual void SetUpdateFunc();
	// 객체가 렌더링 버퍼에 추가될 때, 풀로부터 할당받은 structured buffer에 값을 삽입한다.
	virtual void PushFunc(void* structuredBuffer) const;

private:
	// 렌더링 버퍼에 이 객체를 추가한다.
	void PushRender();
	void Pop() { mIsPushed = false; }

	// 정적 객체 업데이트 (update 실행 x, 렌더링용)
	void UpdateStatic() { Pop(); }
	// 동적 객체 업데이트 (update 실행 o)
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