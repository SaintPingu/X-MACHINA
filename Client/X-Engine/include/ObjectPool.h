#pragma once

#include "UploadBuffer.h"

#pragma region ClassForwardDecl
class MasterModel;
class Transform;
class InstObject;
#pragma endregion




#pragma region Class
// �ν��Ͻ� ��ü���� �����ϱ� ���� ���� (��ü �޸� �Ҵ�)
// �� ��ü�� �ƴ� Ǯ���� �ϳ��� ���� ������, �� ��ü�� transform ������ �޾ƿ� �ν��Ͻ� �������Ѵ�.
class ObjectPool {

protected:
	int mCurrBuffIdx{};

private:
	bool mIsStatic = false;
	sptr<const MasterModel> mMasterModel{};				// ������ ��
	std::vector<const Transform*> mMergedTransform{};	// ������ ���� transform ���� (caching)

	std::vector<sptr<InstObject>>			mObjectPool{};			// all objects
	std::unordered_set<const InstObject*>	mRenderedObjects{};		// rendered objects
	std::unordered_set<int>					mAvailableObjects{};	// ���(Get) ������ ��ü ����
	std::unordered_set<int>					mActiveObjects{};		// Ȱ��ȭ(Get)��    ��ü ����

	std::vector<uptr<UploadBuffer<InstanceData>>> mInstanceBuffers{};


public:
	// model = ������ ��
	// maxSize = Ǯ �ִ� ũ��
	ObjectPool(rsptr<const MasterModel> model, int maxSize);
	virtual ~ObjectPool() = default;

	int GetInstanceCnt() const										{ return mCurrBuffIdx; }
	const std::vector<const Transform*>& GetMergedTransform() const { return mMergedTransform; }

	void SetStatic() { mIsStatic = true; }

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

		if (mObjectPool.front()->GetType() == ObjectType::Static) {
			mIsStatic = true;
		}
	}

	// ��ü�� �Ҵ�޴´�. (enbale==true�� OnEnable()�� ȣ���Ѵ�)
	sptr<InstObject> Get(bool enable = true);
	std::vector<sptr<InstObject>> GetMulti(size_t cnt, bool enable = true);

	// object ��ü�� Ǯ�� ��ȯ�Ѵ�.
	void Return(InstObject* object);

	// command list�� structured buffer�� Set�Ѵ�.
	void UpdateShaderVars() const;

	// �������� ��ü�� �߰��Ѵ�.
	void PushRender(const InstObject* object);
	void Render();
	void PushRenderAllObjects();

	// Ȱ��ȭ��(Get) ��ü�鿡 ���� [func]�� �����Ѵ�.
	void DoActiveObjects(std::function<void(rsptr<InstObject>)> func);
	// pool ���� ��� ��ü�鿡 ���� [func]�� �����Ѵ�.
	void DoAllObjects(std::function<void(rsptr<InstObject>)> func);

	// �� ����
	void ChangeModel(const std::string& modelName);
	void ChangeModel(rsptr<const MasterModel> model);


protected:
	// ������ ����[mSBMap_Inst]�� ��ü�� �� �߰��� �� ������ false�� ��ȯ�Ѵ�.
	bool CanPush() { return mCurrBuffIdx < mObjectPool.size(); }

private:
	void EndRender();
};
#pragma endregion