#include "stdafx.h"
#include "Instancing.h"
#include "DXGIMgr.h"

#include "Scene.h"
#include "Model.h"
#include "Object.h"
#include "Shader.h"


ObjectInstBuffer::ObjectInstBuffer()
{
}


void ObjectInstBuffer::SetModel(rsptr<const MasterModel> model)
{
	mMasterModel = model;
	Transform::MergeTransform(mMergedTransform, mMasterModel->GetTransform());
}

void ObjectInstBuffer::CreateShaderVars(int objectCount)
{
	mObjectCnt = objectCount;
	D3DUtil::CreateBufferResource(NULL, sizeof(*mSBMap_Inst) * mObjectCnt, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mSB_Inst);
	mSB_Inst->Map(0, NULL, (void**)&mSBMap_Inst);
}

void ObjectInstBuffer::UpdateShaderVars() const
{
	cmdList->SetGraphicsRootShaderResourceView(scene->GetRootParamIndex(RootParam::Instancing), mSB_Inst->GetGPUVirtualAddress());
}

void ObjectInstBuffer::PushObject(const InstObject* object)
{
	assert(mCurrBuffIdx < mObjectCnt);

	XMStoreFloat4x4(&mSBMap_Inst[mCurrBuffIdx++].LocalTransform, XMMatrixTranspose(_MATRIX(object->GetWorldTransform())));
}

void ObjectInstBuffer::Render()
{
	if (mMasterModel) {
		mMasterModel->Render(this);
	}

	ResetBuffer();
}