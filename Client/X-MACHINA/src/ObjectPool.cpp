#include "stdafx.h"
#include "ObjectPool.h"

#include "Scene.h"
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

sptr<InstObject> ObjectPool::Get(bool enable) const
{
	// [mAvailableObjects]에서 사용 가능한 id를 얻어 객체를 반환한다.
	if (!mAvailableObjects.empty()) {
		const int id = *mAvailableObjects.begin();

		mAvailableObjects.erase(id);
		mActiveObjects.insert(id);		// 활성화된 객체 집합에 id를 추가한다.
		if (enable) {
			mObjectPool[id]->OnEnable();
		}

		return mObjectPool[id];
	}

	// Log here (pool is full)
	return nullptr;
}

void ObjectPool::Return(InstObject* object)
{
	// [object]의 id를 받아 [mAvailableObjects]에 추가하고 OnDisable()을 호출한다.
	const int id = object->GetPoolID();
	mAvailableObjects.insert(id);
	mActiveObjects.erase(id);			// 활성화된 객체 집합에서 id를 제거한다.
	object->OnDisable();
}


void ObjectPool::UpdateShaderVars() const
{
	scene->SetGraphicsRootShaderResourceView(RootParam::Instancing, mSB_Inst->GetGPUVirtualAddress());
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
	// 렌더링 모델만 인스턴싱으로 렌더링한다.
	if (mMasterModel) {
		mMasterModel->Render(this);
	}

	ResetBuffer();
}

void ObjectPool::DoActiveObjects(std::function<void(rsptr<InstObject>)> func)
{
	std::unordered_set<int> activeObjects = mActiveObjects;	// func 실행 중 객체가 Return()할 수 있기 때문에 복사본으로 루프를 돌도록 한다.

	for (const int id : activeObjects) {
		func(mObjectPool[id]);
	}
}

void ObjectPool::CreateShaderVars()
{
	D3DUtil::CreateBufferResource(nullptr, mStructSize * mObjectPool.size(), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mSB_Inst);
	mSB_Inst->Map(0, nullptr, (void**)&mSBMap_Inst);
}
