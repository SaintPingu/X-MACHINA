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
#pragma region Init_ParticleSystem
	mParticleSystemData.WorldPos	= mObject->GetPosition();
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
#pragma endregion

#pragma region Init_Particles
	mParticles = std::make_unique<UploadBuffer<ParticleData>>(device.Get(), mParticleSystemData.MaxCount, false);
	for (int i = 0; i < mParticleSystemData.MaxCount; ++i) {
		ParticleData particleData;
		particleData.LocalPos.x = 0;
		particleData.LocalPos.y = 0;
		particleData.LocalPos.z = 0;

		particleData.WorldDir.x = Math::RandF(-1.f, 1.f);
		particleData.WorldDir.y = Math::RandF(-1.f, 1.f);
		particleData.WorldDir.z = Math::RandF(-1.f, 1.f);

		particleData.LifeTime = 1.f;
		particleData.Alive = 0;
		mParticles->CopyData(i, particleData);
	}
#pragma endregion
}

void ParticleSystem::Update()
{
	mParticleSystemData.WorldPos = mObject->GetPosition();
	mParticleSystemData.AccTime += DeltaTime();
	mParticleSystemData.DeltaTime = DeltaTime();

	mParticleSystemData.AddCount = 0;
	if (mCreateInterval < mParticleSystemData.AccTime) {
		mParticleSystemData.AccTime = mParticleSystemData.AccTime - mCreateInterval;
		mParticleSystemData.AddCount = 1;
	}
}

void ParticleSystem::OnDestroy()
{
	frmResMgr->ReturnIndex(mParticleSystemIndex, BufferType::ParticleSystem);
	frmResMgr->ReturnIndex(mParticleSystemIndex, BufferType::ParticleShared);
}

void ParticleSystem::Render()
{
	UpdateComputeShaderVars();
	cmdList->Dispatch(1, 1, 1);

	UpdateGraphicsShaderVars();
	res->Get<ModelObjectMesh>("Point")->RenderInstanced(mParticleSystemData.MaxCount);
}

void ParticleSystem::UpdateComputeShaderVars()
{
	res->Get<Shader>("ComputeParticle")->Set();

	frmResMgr->CopyData(mParticleSystemIndex, mParticleSystemData);
	cmdList->SetComputeRootUnorderedAccessView(dxgi->GetParticleComputeRootParamIndex(RootParam::ComputeParticle), mParticles->Resource()->GetGPUVirtualAddress());
	cmdList->SetComputeRoot32BitConstants(dxgi->GetParticleComputeRootParamIndex(RootParam::ParticleIndex), 1, &mParticleSystemIndex, 0);
}

void ParticleSystem::UpdateGraphicsShaderVars()
{
	res->Get<Shader>("GraphicsParticle")->Set();

	cmdList->SetGraphicsRootShaderResourceView(dxgi->GetGraphicsRootParamIndex(RootParam::Particle), mParticles->Resource()->GetGPUVirtualAddress());
}

#pragma endregion

