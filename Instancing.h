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
// 인스턴싱 객체들을 관리하기 위한 버퍼 (객체 메모리 할당)
// 각 객체가 아닌 풀에서 하나의 모델을 가지고, 각 객체의 transform 정보만 받아와 인스턴싱 렌더링한다.
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

	// 렌더링할 객체를 추가한다.
	void PushObject(const InstObject* object);
	void Render();

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }

	void CreateShaderVars(int objectCount);
};
#pragma endregion