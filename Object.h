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
// ǥ�� �ν��Ͻ� StructuredBuffer
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
	bool mIsInstancing{ false };	// �ν��Ͻ� ��ü�ΰ�?

private:
	bool mIsActive{ true };			// Ȱ��ȭ�Ǿ� �ִ°�? (Update, Render)
	bool mIsFlyable{ false };		// �� �� �ִ°�?
	bool mIsDrawBounding{ false };	// collision bounds�� �׸��°�?

	std::unordered_set<int> mGridIndices{};				// ���� Grid indices (�浹��)
	int mCurrGridIndex{ -1 };							// ���� ��ġ�� Grid index

	std::vector<const Transform*> mMergedTransform{};	// ��� ������ transfom (���� ������ ����)

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

	// �ֻ���(��ǥ) �ؽ��ĸ� ��ȯ�Ѵ�.
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

	// [frameName]�� Transform�� ���� �������� ã�� ��ȯ�Ѵ� (������ nullptr)
	Transform* FindFrame(const std::string& frameName);

private:
	// ��ü�� ��ġ(pos)�� ���鿡 ���δ�.
	void AttachToGround();
	// ��ü�� ������ ���⿡ �°� �ٵ��� �Ѵ�.
	void TiltToGround();
};





// instanced GameObject
// ���� ������ �ʴ´�.
class InstObject : public GameObject {
private:
	using base = GameObject;

	using GameObject::Render;

private:
	sptr<ObjectInstBuffer> mBuffer{};

	std::function<void()> mUpdateFunc{};

	bool mIsPushed{ false };	// buffer�� �� ��ü�� �־��°�?

public:
	InstObject()          = default;
	virtual ~InstObject() = default;

	void SetBuffer(rsptr<ObjectInstBuffer> buffer);

public:
	virtual void Render() override { Push(); }
	virtual void Update() override { mUpdateFunc(); }

private:
	// �ν��Ͻ� ���ۿ� �� ��ü�� �߰��Ѵ�.
	void Push();
	void Pop() { mIsPushed = false; }

	// ���� ��ü ������Ʈ (update ���� x, ��������)
	void UpdateStatic();
	// ���� ��ü ������Ʈ (update ���� o)
	void UpdateDynamic();
};





// �ν��Ͻ� ��ü���� ��� ���� ����
// ��ü�� �ƴ� ���ۿ��� ���� ������, �� ��ü�� transform ������ �޾ƿ� �������Ѵ�.
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

	// �������� ��ü�� �߰��Ѵ�.
	void PushObject(const InstObject* object);
	void Render();

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }
};
#pragma endregion