#include "stdafx.h"
#include "Instancing.h"
#include "DXGIMgr.h"

#include "Scene.h"
#include "Model.h"
#include "Object.h"
#include "Shader.h"



ObjectPool::ObjectPool(rsptr<const MasterModel> model, int maxSize)
	:
	mMasterModel(model)
{
	Transform::MergeTransform(mMergedTransform, mMasterModel->GetTransform());
	mObjectPool.resize(maxSize);
	int id{};
	for (auto& object : mObjectPool) {
		mAvailableObjects.insert(id);
		object = std::make_shared<InstObject>();
		object->SetBuffer(this, id);
		object->Disable();

		++id;
	}

	CreateShaderVars(maxSize);
}

sptr<InstObject> ObjectPool::Get() const
{
	int id = *mAvailableObjects.begin();

	if (!mAvailableObjects.empty()) {
		mAvailableObjects.erase(id);
		mObjectPool[id]->Enable(true);

		return mObjectPool[id];
	}

	throw std::runtime_error("pool is full");
	//return nullptr;	// throw보단 nullptr 처리
}

void ObjectPool::Return(InstObject* object)
{
	mAvailableObjects.insert(object->GetPoolID());
	object->Disable(true);
}


void ObjectPool::CreateShaderVars(int objectCount)
{
	mObjectCnt = objectCount;
	D3DUtil::CreateBufferResource(NULL, sizeof(*mSBMap_Inst) * mObjectCnt, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mSB_Inst);
	mSB_Inst->Map(0, NULL, (void**)&mSBMap_Inst);
}

void ObjectPool::UpdateShaderVars() const
{
	cmdList->SetGraphicsRootShaderResourceView(scene->GetRootParamIndex(RootParam::Instancing), mSB_Inst->GetGPUVirtualAddress());
}

void ObjectPool::PushObject(const InstObject* object)
{
	assert(mCurrBuffIdx < mObjectCnt);

	XMStoreFloat4x4(&mSBMap_Inst[mCurrBuffIdx++].LocalTransform, XMMatrixTranspose(_MATRIX(object->GetWorldTransform())));
}

void ObjectPool::Render()
{
	if (mMasterModel) {
		mMasterModel->Render(this);
	}

	ResetBuffer();
}