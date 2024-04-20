#include "EnginePch.h"
#include "ObjectPool.h"

#include "DXGIMgr.h"
#include "Model.h"
#include "Object.h"



ObjectPool::ObjectPool(rsptr<const MasterModel> model, int maxSize, size_t structSize)
	:
	mMasterModel(model),
	mStructSize(structSize),
	mObjectPool(maxSize)
{
	Transform::MergeTransform(mMergedTransform, mMasterModel->GetTransform());

	CreateShaderVars();
}

sptr<InstObject> ObjectPool::Get(bool enable)
{
	// [mAvailableObjects]���� ��� ������ id�� ��� ��ü�� ��ȯ�Ѵ�.
	if (!mAvailableObjects.empty()) {
		const int id = *mAvailableObjects.begin();

		mAvailableObjects.erase(id);
		mActiveObjects.insert(id);		// Ȱ��ȭ�� ��ü ���տ� id�� �߰��Ѵ�.
		if (enable) {
			mObjectPool[id]->OnEnable();
		}

		return mObjectPool[id];
	}

	// Log here (pool is full)
	return nullptr;
}

std::vector<sptr<InstObject>> ObjectPool::GetMulti(size_t cnt, bool enable)
{
	std::vector<sptr<InstObject>> result(cnt);

	// [mAvailableObjects]���� ��� ������ id���� ��� ��ü���� ��ȯ�Ѵ�.
	size_t i = 0;
	for (; i < cnt; ++i) {
		if (mAvailableObjects.empty()) {
			break;
		}

		const int id = *mAvailableObjects.begin();

		mAvailableObjects.erase(id);
		mActiveObjects.insert(id);		// Ȱ��ȭ�� ��ü ���տ� id�� �߰��Ѵ�.
		if (enable) {
			mObjectPool[id]->OnEnable();
		}

		result[i] = mObjectPool[id];
	}

	result.resize(i);

	return result;

}

void ObjectPool::Return(InstObject* object)
{
	// [object]�� id�� �޾� [mAvailableObjects]�� �߰��ϰ� OnDisable()�� ȣ���Ѵ�.
	const int id = object->GetPoolID();
	mAvailableObjects.insert(id);
	mActiveObjects.erase(id);			// Ȱ��ȭ�� ��ü ���տ��� id�� �����Ѵ�.
	object->OnDisable();
}


void ObjectPool::UpdateShaderVars() const
{
	DXGIMgr::I->SetGraphicsRootShaderResourceView(RootParam::Instancing, mSB_Inst->GetGPUVirtualAddress());
}

void ObjectPool::PushRender(const InstObject* object)
{
	if (!CanPush()) {
		// Log here
		return;
	}

	void* buffer = static_cast<char*>(mSBMap_Inst) + (mStructSize * mCurrBuffIdx++);
	object->PushFunc(buffer);
}


void ObjectPool::Render()
{
	// ������ �𵨸� �ν��Ͻ����� �������Ѵ�.
	if (mMasterModel) {
		mMasterModel->Render(this);
	}

	ResetBuffer();
}

void ObjectPool::DoActiveObjects(std::function<void(rsptr<InstObject>)> func)
{
	std::unordered_set<int> activeObjects = mActiveObjects;	// func ���� �� ��ü�� Return()�� �� �ֱ� ������ ���纻���� ������ ������ �Ѵ�.

	for (const int id : activeObjects) {
		func(mObjectPool[id]);
	}
}

void ObjectPool::DoAllObjects(std::function<void(rsptr<InstObject>)> func)
{
	for (const auto& object : mObjectPool) {
		func(object);
	}
}

void ObjectPool::CreateShaderVars()
{
	D3DUtil::CreateBufferResource(nullptr, mStructSize * mObjectPool.size(), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mSB_Inst);
	mSB_Inst->Map(0, nullptr, (void**)&mSBMap_Inst);
}
