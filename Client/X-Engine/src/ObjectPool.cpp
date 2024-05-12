#include "EnginePch.h"
#include "ObjectPool.h"

#include "DXGIMgr.h"
#include "Model.h"
#include "Object.h"

#include "FrameResource.h"

ObjectPool::ObjectPool(rsptr<const MasterModel> model, int maxSize)
	:
	mMasterModel(model),
	mObjectPool(maxSize)
{
	Transform::MergeTransform(mMergedTransform, mMasterModel->GetTransform());

	mInstanceBuffers.resize(FrameResourceMgr::mkFrameResourceCount);
	for (auto& buffer : mInstanceBuffers) {
		buffer = std::make_unique<UploadBuffer<InstanceData>>(DEVICE.Get(), maxSize, false);
	}
}

sptr<InstObject> ObjectPool::Get(bool enable)
{
	// [mAvailableObjects]���� ��� ������ id�� ��� ��ü�� ��ȯ�Ѵ�.
	if (!mAvailableObjects.empty()) {
		const int id = *mAvailableObjects.begin();

		mAvailableObjects.erase(id);
		mActiveObjects.insert(id);		// Ȱ��ȭ�� ��ü ���տ� id�� �߰��Ѵ�.
		if (enable) {
			mObjectPool[id]->SetActive(true);
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
			mObjectPool[id]->SetActive(true);
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
	object->SetActive(false);
}

void ObjectPool::UpdateShaderVars() const
{
	DXGIMgr::I->SetGraphicsRootShaderResourceView(RootParam::Instancing, FrameResourceMgr::GetBufferGpuAddr(0, mInstanceBuffers[CURR_FRAME_INDEX].get()));
}

void ObjectPool::PushRender(const InstObject* object)
{
	if (mRenderedObjects.contains(object)) {
		return;
	}

	if (!CanPush()) {
		// Log here
		return;
	}

	mRenderedObjects.insert(object);

	object->PushFunc(mCurrBuffIdx++, mInstanceBuffers[CURR_FRAME_INDEX].get());
}

void ObjectPool::Render()
{
	// ������ �𵨸� �ν��Ͻ����� �������Ѵ�.
	if (mMasterModel) {
		mMasterModel->Render(this);
	}

	EndRender();
}

void ObjectPool::DoActiveObjects(std::function<void(rsptr<InstObject>)> func)
{
	if (mIsStatic) {
		return;
	}

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

void ObjectPool::EndRender()
{
	mCurrBuffIdx = 0;
	mRenderedObjects.clear();
}