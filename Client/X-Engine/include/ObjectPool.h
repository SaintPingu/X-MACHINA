#pragma once

#pragma region ClassForwardDecl
class MasterModel;
class Transform;
class InstObject;
#pragma endregion


#pragma region Struct
// ǥ�� �ν��Ͻ� StructuredBuffer
struct SB_StandardInst {
	Matrix LocalTransform{};
};

// Color�� ������ ��ü�� �ν��Ͻ� StructuredBuffer
struct SB_ColorInst {
	Matrix	LocalTransform{};
	Vec4	Color{};
};
#pragma endregion


#pragma region Class
// �ν��Ͻ� ��ü���� �����ϱ� ���� ���� (��ü �޸� �Ҵ�)
// �� ��ü�� �ƴ� Ǯ���� �ϳ��� ���� ������, �� ��ü�� transform ������ �޾ƿ� �ν��Ͻ� �������Ѵ�.
class ObjectPool {
protected:
	int mCurrBuffIdx{};

private:
	const sptr<const MasterModel> mMasterModel{};		// ������ ��
	std::vector<const Transform*> mMergedTransform{};	// ������ ���� transform ���� (caching)

	std::vector<sptr<InstObject>>	mObjectPool{};			// all objects
	mutable std::unordered_set<int>	mAvailableObjects{};	// ���(Get) ������ ��ü ����
	mutable std::unordered_set<int>	mActiveObjects{};		// Ȱ��ȭ(Get)��    ��ü ����

	ComPtr<ID3D12Resource>	mSB_Inst{};		// StructuredBuffer for instance
	void*					mSBMap_Inst{};	// mapped StructuredBuffer
	const size_t			mStructSize{};  // sizeof(structured buffer)

public:
	// model = ������ ��
	// maxSize = Ǯ �ִ� ũ��
	// structSize = structured buffer�� ũ�� [sizeof(SB_StandardInst)]
	ObjectPool(rsptr<const MasterModel> model, int maxSize, size_t structSize);
	virtual ~ObjectPool() = default;

	int GetInstanceCnt() const										{ return mCurrBuffIdx; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }

public:
	// Ǯ�� ��ü�� ���� �� �ʱ�ȭ�Ѵ�. ��� ��ü�鿡 ���� [objectInitFunc]�� �����Ѵ�.
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

	// ��ü�� �Ҵ�޴´�. (enbale==true�� OnEnable()�� ȣ���Ѵ�)
	sptr<InstObject> Get(bool enable = true) const;
	std::vector<sptr<InstObject>> GetMulti(size_t cnt, bool enable = true) const;

	// object ��ü�� Ǯ�� ��ȯ�Ѵ�.
	void Return(InstObject* object);

	// command list�� structured buffer�� Set�Ѵ�.
	void UpdateShaderVars() const;

	// �������� ��ü�� �߰��Ѵ�.
	void PushRender(const InstObject* object);
	void Render();

	// Ȱ��ȭ��(Get) ��ü�鿡 ���� [func]�� �����Ѵ�.
	void DoActiveObjects(std::function<void(rsptr<InstObject>)> func);
	// pool ���� ��� ��ü�鿡 ���� [func]�� �����Ѵ�.
	void DoAllObjects(std::function<void(rsptr<InstObject>)> func);

protected:
	// ������ ����[mSBMap_Inst]�� ��ü�� �� �߰��� �� ������ false�� ��ȯ�Ѵ�.
	bool CanPush() { return mCurrBuffIdx < mObjectPool.size(); }

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }

	// create structured buffer resource
	void CreateShaderVars();
};
#pragma endregion