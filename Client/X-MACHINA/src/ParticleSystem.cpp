#include "stdafx.h"
#include "ParticleSystem.h"
#include "Timer.h"
#include "ResourceMgr.h"
#include "Mesh.h"
#include "DXGIMgr.h"
#include "Shader.h"
#include "Texture.h"
#include "FrameResource.h"



#pragma region ParticleSystem
void ParticleSystem::Awake()
{
	base::Awake();

	mParticleSystemData.AddCount	= 0;
	mParticleSystemData.MaxCount	= 1000;
	mParticleSystemData.DeltaTime	= 0.f;
	mParticleSystemData.AccTime		= 0.f;
	mParticleSystemData.MinLifeTime = 0.5f;
	mParticleSystemData.MaxLifeTime = 5.f;
	mParticleSystemData.MinSpeed	= 100;
	mParticleSystemData.MaxSpeed	= 50;
	mParticleSystemData.StartScale	= 10.f;
	mParticleSystemData.EndScale	= 0.f;

	mParticles = std::make_unique<UploadBuffer<ParticleData>>(device.Get(), mParticleSystemData.MaxCount, false);
	
	// TODO : shape에 따른 파티클들의 시작 지점과 방향 설정
	for (int i = 0; i < mParticleSystemData.MaxCount; ++i) {
		ParticleData particleData;
		particleData.WorldPos.x = -10.f;
		particleData.WorldPos.y = -10.f;
		particleData.WorldPos.z = -10.f;
		mParticles->CopyData(i, particleData);
	}
}


void ParticleSystem::Update()
{
	mParticleSystemData.AccTime += DeltaTime();
	mParticleSystemData.DeltaTime = DeltaTime();

	mParticleSystemData.AddCount = 0;
	if (mCreateInterval < mParticleSystemData.AccTime) {
		mParticleSystemData.AccTime = mParticleSystemData.AccTime - mCreateInterval;
		mParticleSystemData.AddCount = 1;
	}
}
#pragma endregion





#pragma region ParticleSystemObject
ParticleSystemObject::ParticleSystemObject() : Object()
{
	mParticleSystem = AddComponent<ParticleSystem>();
}

void ParticleSystemObject::Awake()
{
	base::Awake();
}

void ParticleSystemObject::Update()
{
	base::Update();					// update base
	mParticleSystem->Update();		// update particle system 
}


void ParticleSystemObject::OnDestroy()
{
	base::OnDestroy();
	frmResMgr->ReturnIndex(mParticleSystemIndex, BufferType::ParticleSystem);
	frmResMgr->ReturnIndex(mParticleSystemIndex, BufferType::ParticleShared);
}

void ParticleSystemObject::UpdateComputeShaderVars()
{
	res->Get<Shader>("ComputeParticle")->Set();

	frmResMgr->CopyData(mParticleSystemIndex, mParticleSystem->mParticleSystemData);

	cmdList->SetComputeRootUnorderedAccessView(dxgi->GetParticleComputeRootParamIndex(RootParam::ComputeParticle), mParticleSystem->mParticles->Resource()->GetGPUVirtualAddress());
	cmdList->SetComputeRoot32BitConstants(dxgi->GetParticleComputeRootParamIndex(RootParam::ParticleIndex), 1, &mParticleSystemIndex, 0);

	cmdList->Dispatch(1, 1, 1);
}

void ParticleSystemObject::UpdateGraphicsShaderVars()
{
	res->Get<Shader>("GraphicsParticle")->Set();

	Transform::UpdateShaderVars();

	cmdList->SetGraphicsRootShaderResourceView(dxgi->GetGraphicsRootParamIndex(RootParam::Particle), mParticleSystem->mParticles->Resource()->GetGPUVirtualAddress());
}

void ParticleSystemObject::Render()
{
	UpdateComputeShaderVars();
	UpdateGraphicsShaderVars();
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	res->Get<ModelObjectMesh>("Point")->RenderInstanced(mParticleSystem->mParticleSystemData.MaxCount);
}



#pragma endregion

