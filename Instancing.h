#pragma once

#pragma region ClassForwardDecl
class MasterModel;
class Transform;
class InstObject;
#pragma endregion


#pragma region Struct
// ǥ�� �ν��Ͻ� StructuredBuffer
struct SB_StandardInst {
	Vec4x4 LocalTransform{};
};
#pragma endregion


#pragma region Class
// �ν��Ͻ� ��ü���� �����ϱ� ���� ���� (��ü �޸� �Ҵ�)
// �� ��ü�� �ƴ� Ǯ���� �ϳ��� ���� ������, �� ��ü�� transform ������ �޾ƿ� �ν��Ͻ� �������Ѵ�.
class ObjectPool {
protected:
	int mCurrBuffIdx{};

private:
	int mObjectCnt{};
	sptr<const MasterModel> mMasterModel{};

	mutable std::unordered_set<int>	mAvailableObjects{};	// for chaching
	std::vector<sptr<InstObject>>	mObjectPool{};
	std::vector<const Transform*>	mMergedTransform{};

	ComPtr<ID3D12Resource>			mSB_Inst{};		// StructuredBuffer for instance
	SB_StandardInst*				mSBMap_Inst{};	// mapped StructuredBuffer

public:
	ObjectPool(rsptr<const MasterModel> model, int maxSize);
	virtual ~ObjectPool() = default;

	int GetInstanceCnt() const { return mCurrBuffIdx; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }

public:
	sptr<InstObject> Get() const;
	void Return(InstObject* object);

	void UpdateShaderVars() const;

	// �������� ��ü�� �߰��Ѵ�.
	void PushObject(const InstObject* object);
	void Render();

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }

	void CreateShaderVars(int objectCount);
};
#pragma endregion