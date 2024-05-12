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
	// [mAvailableObjects]에서 사용 가능한 id를 얻어 객체를 반환한다.
	if (!mAvailableObjects.empty()) {
		const int id = *mAvailableObjects.begin();

		mAvailableObjects.erase(id);
		mActiveObjects.insert(id);		// 활성화된 객체 집합에 id를 추가한다.
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

	// [mAvailableObjects]에서 사용 가능한 id들을 얻어 객체들을 반환한다.
	size_t i = 0;
	for (; i < cnt; ++i) {
		if (mAvailableObjects.empty()) {
			break;
		}

		const int id = *mAvailableObjects.begin();

		mAvailableObjects.erase(id);
		mActiveObjects.insert(id);		// 활성화된 객체 집합에 id를 추가한다.
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
	// [object]의 id를 받아 [mAvailableObjects]에 추가하고 OnDisable()을 호출한다.
	const int id = object->GetPoolID();
	mAvailableObjects.insert(id);
	mActiveObjects.erase(id);			// 활성화된 객체 집합에서 id를 제거한다.
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
	// 렌더링 모델만 인스턴싱으로 렌더링한다.
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

	std::unordered_set<int> activeObjects = mActiveObjects;	// func 실행 중 객체가 Return()할 수 있기 때문에 복사본으로 루프를 돌도록 한다.

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