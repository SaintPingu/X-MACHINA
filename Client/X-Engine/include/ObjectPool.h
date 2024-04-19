#pragma once

#pragma region ClassForwardDecl
class MasterModel;
class Transform;
class InstObject;
#pragma endregion


#pragma region Struct
// 표준 인스턴싱 StructuredBuffer
struct SB_StandardInst {
	Matrix LocalTransform{};
};

// Color를 가지는 객체의 인스턴싱 StructuredBuffer
struct SB_ColorInst {
	Matrix	LocalTransform{};
	Vec4	Color{};
};
#pragma endregion


#pragma region Class
// 인스턴싱 객체들을 관리하기 위한 버퍼 (객체 메모리 할당)
// 각 객체가 아닌 풀에서 하나의 모델을 가지고, 각 객체의 transform 정보만 받아와 인스턴싱 렌더링한다.
class ObjectPool {
protected:
	int mCurrBuffIdx{};

private:
	const sptr<const MasterModel> mMasterModel{};		// 렌더링 모델
	std::vector<const Transform*> mMergedTransform{};	// 렌더링 모델의 transform 구조 (caching)

	std::vector<sptr<InstObject>>	mObjectPool{};			// all objects
	mutable std::unordered_set<int>	mAvailableObjects{};	// 사용(Get) 가능한 객체 집합
	mutable std::unordered_set<int>	mActiveObjects{};		// 활성화(Get)된    객체 집합

	ComPtr<ID3D12Resource>	mSB_Inst{};		// StructuredBuffer for instance
	void*					mSBMap_Inst{};	// mapped StructuredBuffer
	const size_t			mStructSize{};  // sizeof(structured buffer)

public:
	// model = 랜더링 모델
	// maxSize = 풀 최대 크기
	// structSize = structured buffer의 크기 [sizeof(SB_StandardInst)]
	ObjectPool(rsptr<const MasterModel> model, int maxSize, size_t structSize);
	virtual ~ObjectPool() = default;

	int GetInstanceCnt() const										{ return mCurrBuffIdx; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }

public:
	// 풀링 객체를 생성 및 초기화한다. 모든 객체들에 대해 [objectInitFunc]을 실행한다.
	template<typename T, typename = std::enable_if_t<std::is_base_of_v<InstObject, T>>>
	void CreateObjects(const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr)
	{
		int id{};
		for (auto& object : mObjectPool) {
			mAvailableObjects.insert(id);
			object = std::make_shared<T>(this, id);
			object->SetModel(mMasterModel);
			if (objectInitFunc) {
				objectInitFunc(object);
			}
			object->SetUpdateFunc();

			++id;
		}
	}

	// 객체를 할당받는다. (enbale==true시 OnEnable()을 호출한다)
	sptr<InstObject> Get(bool enable = true) const;
	std::vector<sptr<InstObject>> GetMulti(size_t cnt, bool enable = true) const;

	// object 객체를 풀에 반환한다.
	void Return(InstObject* object);

	// command list에 structured buffer를 Set한다.
	void UpdateShaderVars() const;

	// 렌더링할 객체를 추가한다.
	void PushRender(const InstObject* object);
	void Render();

	// 활성화된(Get) 객체들에 대해 [func]을 실행한다.
	void DoActiveObjects(std::function<void(rsptr<InstObject>)> func);
	// pool 내의 모든 객체들에 대해 [func]을 실행한다.
	void DoAllObjects(std::function<void(rsptr<InstObject>)> func);

protected:
	// 렌더링 버퍼[mSBMap_Inst]에 객체를 더 추가할 수 없으면 false를 반환한다.
	bool CanPush() { return mCurrBuffIdx < mObjectPool.size(); }

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }

	// create structured buffer resource
	void CreateShaderVars();
};
#pragma endregion