#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class MasterModel;
class Texture;
class ObjectInstBuffer;
class ObjectCollider;
#pragma endregion


#pragma region Struct
// 표준 인스턴싱 StructuredBuffer
struct SB_StandardInst {
	Vec4x4 LocalTransform{};
};
#pragma endregion


#pragma region Class
// base class for all entities in Scene
class GameObject : public Object {
	using base = Object;
	using Transform::ReturnToPrevTransform;

public:
	sptr<const MasterModel> mMasterModel{};

protected:
	bool mIsInstancing{ false };	// 인스턴싱 객체인가?

private:
	bool mIsActive{ true };			// 활성화되어 있는가? (Update, Render)
	bool mIsFlyable{ false };		// 날 수 있는가?
	bool mIsDrawBounding{ false };	// collision bounds를 그리는가?

	std::unordered_set<int> mGridIndices{};				// 인접 Grid indices (충돌됨)
	int mCurrGridIndex{ -1 };							// 현재 위치의 Grid index

	std::vector<const Transform*> mMergedTransform{};	// 모든 계층의 transfom (빠른 접근을 위해)

	sptr<ObjectCollider> mCollider{};

public:
#pragma region C/Dtor
	GameObject();
	virtual ~GameObject() = default;
#pragma endregion

#pragma region Getter
	int GetGridIndex() const { return mCurrGridIndex; }
	const std::unordered_set<int>& GetGridIndices() const { return mGridIndices; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }

	// 최상위(대표) 텍스쳐를 반환한다.
	rsptr<Texture> GetTexture() const;

	rsptr<ObjectCollider> GetCollider() const { return mCollider; }
#pragma endregion

#pragma region Setter
	bool IsActive() const { return mIsActive; }
	bool IsTransparent() const { return mLayer == ObjectLayer::Transparent; }
	bool IsInstancing() const { return mIsInstancing; }

	void SetInstancing() { mIsInstancing = true; }
	void SetFlyable(bool isFlyable) { mIsFlyable = isFlyable; }

	void SetGridIndex(int index) { mCurrGridIndex = index; }
	void SetGridIndices(const std::unordered_set<int>& indices) { mGridIndices = indices; }

	void SetModel(rsptr<const MasterModel> model);
#pragma endregion

public:
	virtual void Render();

	// render collision bounds
	virtual void RenderBounds();

	virtual void Update();

	virtual void Enable(bool isUpdateObjectGrid = true);
	virtual void Disable(bool isUpdateObjectGrid = true);

	void ToggleDrawBoundings() { mIsDrawBounding = !mIsDrawBounding; }
	void ClearGridIndices() { mGridIndices.clear(); }

	// [frameName]의 Transform을 계층 구조에서 찾아 반환한다 (없으면 nullptr)
	Transform* FindFrame(const std::string& frameName);

private:
	// 객체의 위치(pos)를 지면에 붙인다.
	void AttachToGround();
	// 객체를 지면의 기울기에 맞게 붙도록 한다.
	void TiltToGround();
};





// instanced GameObject
// 모델을 가지지 않는다.
class InstObject : public GameObject {
private:
	using base = GameObject;

	using GameObject::Render;

private:
	sptr<ObjectInstBuffer> mBuffer{};

	std::function<void()> mUpdateFunc{};

	bool mIsPushed{ false };	// buffer에 이 객체를 넣었는가?

public:
	InstObject()          = default;
	virtual ~InstObject() = default;

	void SetBuffer(rsptr<ObjectInstBuffer> buffer);

public:
	virtual void Render() override { Push(); }
	virtual void Update() override { mUpdateFunc(); }

private:
	// 인스턴싱 버퍼에 이 객체를 추가한다.
	void Push();
	void Pop() { mIsPushed = false; }

	// 정적 객체 업데이트 (update 실행 x, 렌더링용)
	void UpdateStatic();
	// 동적 객체 업데이트 (update 실행 o)
	void UpdateDynamic();
};





// 인스턴싱 객체들을 담기 위한 버퍼
// 객체가 아닌 버퍼에서 모델을 가지고, 각 객체의 transform 정보만 받아와 렌더링한다.
class ObjectInstBuffer {
protected:
	int mCurrBuffIdx{};
	bool mIsStatic{ true };

private:
	int mObjectCnt{};
	sptr<const MasterModel> mMasterModel{};

	std::vector<const Transform*>	mMergedTransform{};

	ComPtr<ID3D12Resource>			mSB_Inst{};		// StructuredBuffer for instance
	SB_StandardInst*				mSBMap_Inst{};	// mapped StructuredBuffer

public:
	ObjectInstBuffer()          = default;
	virtual ~ObjectInstBuffer() = default;

	bool IsStatic() { return mIsStatic; }
	int GetInstanceCnt() const { return mCurrBuffIdx; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }

	void SetDynamic() { mIsStatic = false; }
	void SetModel(rsptr<const MasterModel> model);

public:
	void CreateShaderVars(int objectCount);
	void UpdateShaderVars() const;

	// 렌더링할 객체를 추가한다.
	void PushObject(const InstObject* object);
	void Render();

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }
};
#pragma endregion