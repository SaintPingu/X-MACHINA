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
	ObjectInstBuffer() = default;
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