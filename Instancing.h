#pragma once

#pragma region ClassForwardDecl
class MasterModel;
class Transform;
class InstObject;
#pragma endregion


#pragma region Struct
// 표준 인스턴싱 StructuredBuffer
struct SB_StandardInst {
	Vec4x4 LocalTransform{};
};
#pragma endregion


#pragma region Class
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

	// 렌더링할 객체를 추가한다.
	void PushObject(const InstObject* object);
	void Render();

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }
};
#pragma endregion