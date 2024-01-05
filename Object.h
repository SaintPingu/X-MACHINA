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

	std::vector<const Transform*> mMergedTransform{};	// 모든 계층의 transfom (빠른 접근을 위한 캐싱)


public:
#pragma region C/Dtor
	GameObject() = default;
	virtual ~GameObject() = default;
#pragma endregion

#pragma region Getter
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }

	// 최상위(대표) 텍스쳐를 반환한다.
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

	// [frameName]의 Transform을 계층 구조에서 찾아 반환한다 (없으면 nullptr)
	Transform* FindFrame(const std::string& frameName);

private:
	// 객체의 위치(pos)를 지면에 붙인다.
	void AttachToGround();
};





class GridObject : public GameObject {
	using base = GameObject;

private:
	std::unordered_set<int> mGridIndices{};				// 인접 Grid indices (충돌됨)
	int mCurrGridIndex = -1;							// 현재 위치의 Grid index
	sptr<ObjectCollider> mCollider{};

	bool mIsDrawBounding = false;	// collision bounds를 그리는가?

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
// 모델을 가지지 않는다.
class InstObject : public GridObject {
private:
	int mPoolID{};
	using base = GridObject;

	using GridObject::Render;

private:
	ObjectPool* mBuffer{};

	std::function<void()> mUpdateFunc{};

	bool mIsPushed{ false };	// buffer에 이 객체를 넣었는가?

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
	// 인스턴싱 버퍼에 이 객체를 추가한다.
	void Push();
	void Pop() { mIsPushed = false; }

	// 정적 객체 업데이트 (update 실행 x, 렌더링용)
	void UpdateStatic();
	// 동적 객체 업데이트 (update 실행 o)
	void UpdateDynamic();
};
#pragma endregion