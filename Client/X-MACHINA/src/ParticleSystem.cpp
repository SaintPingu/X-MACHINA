#include "stdafx.h"
#include "ParticleSystem.h"
#include "Timer.h"
#include "ResourceMgr.h"
#include "Mesh.h"
#include "DXGIMgr.h"
#include "Shader.h"
#include "FrameResource.h"


void ParticleSystem::SetTarget(const std::string& frameName)
{
	Transform* findFrame = mObject->FindFrame(frameName);
	if (findFrame) {
		mTarget = findFrame;
	}
}

void ParticleSystem::Awake()
{
#pragma region Init_ParticleSystem
	frmResMgr->CopyData(mPSIdx, mPSData);
#pragma endregion

#pragma region Init_Particles
	mParticles = std::make_unique<UploadBuffer<ParticleData>>(device.Get(), mPSData.MaxCount, false);
	for (int i = 0; i < mPSData.MaxCount; ++i) {
		ParticleData particleData;
		particleData.LocalPos.x = 0;
		particleData.LocalPos.y = 0;
		particleData.LocalPos.z = 0;

		particleData.WorldDir.x = Math::RandF(-1.f, 1.f);
		particleData.WorldDir.y = Math::RandF(-1.f, 1.f);
		particleData.WorldDir.z = Math::RandF(-1.f, 1.f);

		particleData.LifeTime = Math::RandF(mPSData.MinLifeTime, mPSData.MaxLifeTime);
		mParticles->CopyData(i, particleData);
	}
#pragma endregion
}

void ParticleSystem::Update()
{
	mPSData.WorldPos = mTarget->GetPosition();
	mPSData.AccTime += DeltaTime();
	mPSData.DeltaTime = DeltaTime();

	mPSData.AddCount = 0;
	if (mCreateInterval < mPSData.AccTime) {
		mPSData.AccTime = mPSData.AccTime - mCreateInterval;
		mPSData.AddCount = 1;
	}

	frmResMgr->CopyData(mPSIdx, mPSData);
}

void ParticleSystem::OnDestroy()
{
	frmResMgr->ReturnIndex(mPSIdx, BufferType::ParticleSystem);
	frmResMgr->ReturnIndex(mPSIdx, BufferType::ParticleShared);
}

void ParticleSystem::Render() const
{
	UpdateComputeShaderVars();
	cmdList->Dispatch(1, 1, 1);

	UpdateGraphicsShaderVars();
	res->Get<ModelObjectMesh>("Point")->RenderInstanced(mPSData.MaxCount);
}

void ParticleSystem::UpdateComputeShaderVars() const
{
	res->Get<Shader>("ComputeParticle")->Set();

	cmdList->SetComputeRootUnorderedAccessView(dxgi->GetParticleComputeRootParamIndex(RootParam::ComputeParticle), mParticles->Resource()->GetGPUVirtualAddress());
	cmdList->SetComputeRoot32BitConstants(dxgi->GetParticleComputeRootParamIndex(RootParam::ParticleIndex), 1, &mPSIdx, 0);
}

void ParticleSystem::UpdateGraphicsShaderVars() const
{
	res->Get<Shader>("GraphicsParticle")->Set();

	cmdList->SetGraphicsRootShaderResourceView(dxgi->GetGraphicsRootParamIndex(RootParam::Particle), mParticles->Resource()->GetGPUVirtualAddress());
}

