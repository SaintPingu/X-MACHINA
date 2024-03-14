#include "stdafx.h"
#include "ParticleSystem.h"
#include "Timer.h"
#include "ResourceMgr.h"
#include "Mesh.h"
#include "DXGIMgr.h"
#include "Shader.h"
#include "FrameResource.h"





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region ParticleSystem
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
	frmResMgr->CopyData(mPSIdx, mPSGD);
#pragma endregion

#pragma region Init_Particles
	mParticles = std::make_unique<UploadBuffer<ParticleData>>(device.Get(), mPSGD.MaxCount, false);
#pragma endregion

	Play();
}

void ParticleSystem::Update()
{
#pragma region Check_Looping
	if (!mPSCD.IsLooping && mPSCD.LoopingElapsed >= mPSCD.Duration) {
		Stop();
	}
#pragma endregion

#pragma region Check_StartDelay
	mPSCD.StartElapsed += DeltaTime();
	if (mPSCD.StartElapsed <= mPSCD.StartDelay) {
		return;
	}
#pragma endregion

#pragma region Check_StopDelay
	if (mPSCD.IsStop) {
		mPSCD.StopElapsed += DeltaTime();
		if (mPSCD.StopElapsed >= (!mPSCD.Prewarm ? mPSGD.MaxLifeTime : 0)) {
			SetActive(false);
			pr->RemoveParticleSystem(mPSIdx);
			return;
		}
	}
#pragma endregion

#pragma region Update
	mPSGD.WorldPos = mTarget->GetPosition();
	mPSGD.AccTime += DeltaTime();
	mPSGD.DeltaTime = DeltaTime();
	mPSCD.LoopingElapsed += DeltaTime();

	if (mPSCD.CreateInterval < mPSGD.AccTime) {
		mPSGD.AccTime = mPSGD.AccTime - mPSCD.CreateInterval;
		mPSGD.AddCount = mPSCD.IsStop ? 0 : mPSCD.MaxAddCount;
	}

	frmResMgr->CopyData(mPSIdx, mPSGD);
#pragma endregion
}

void ParticleSystem::OnDestroy()
{
	frmResMgr->ReturnIndex(mPSIdx, BufferType::ParticleSystem);
	frmResMgr->ReturnIndex(mPSIdx, BufferType::ParticleShared);
}

void ParticleSystem::Play()
{
	if (mPSCD.IsStop) {
		SetActive(true);
		pr->AddParticleSystem(shared_from_this());
		mPSCD.LoopingElapsed = 0.f;
		mPSCD.StopElapsed = 0.f;
		mPSCD.StartElapsed = 0.f;
		mPSCD.IsStop = false;
	}
}

void ParticleSystem::Stop()
{
	if (!mPSCD.IsStop) {
		mPSCD.IsStop = true;
	}
}

void ParticleSystem::PlayToggle()
{
	mPSCD.IsStop ? Play() : Stop();
}

void ParticleSystem::ComputeParticleSystem() const
{
	cmdList->SetComputeRootUnorderedAccessView(dxgi->GetParticleComputeRootParamIndex(RootParam::ComputeParticle), mParticles->Resource()->GetGPUVirtualAddress());
	cmdList->SetComputeRoot32BitConstants(dxgi->GetParticleComputeRootParamIndex(RootParam::ParticleIndex), 1, &mPSIdx, 0);

	cmdList->Dispatch(1, 1, 1);
}

void ParticleSystem::RenderParticleSystem() const
{
	cmdList->SetGraphicsRootShaderResourceView(dxgi->GetGraphicsRootParamIndex(RootParam::Particle), mParticles->Resource()->GetGPUVirtualAddress());
	res->Get<ModelObjectMesh>("Point")->RenderInstanced(mPSGD.MaxCount);
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region ParticleRenderer
void ParticleRenderer::Init()
{
	mParticleSystems.reserve(300);
}

void ParticleRenderer::AddParticleSystem(sptr<ParticleSystem> particleSystem)
{
	mParticleSystems.emplace(std::make_pair(particleSystem->GetPSIdx(), particleSystem));
}

void ParticleRenderer::RemoveParticleSystem(int particleSystemIdx)
{
	mParticleSystems.erase(particleSystemIdx);
}

void ParticleRenderer::Render() const
{
	res->Get<Shader>("ComputeParticle")->Set();
	for (const auto& ps : mParticleSystems) {
		ps.second->ComputeParticleSystem();
	}

	res->Get<Shader>("GraphicsParticle")->Set();
	for (const auto& ps : mParticleSystems) {
		ps.second->RenderParticleSystem();
	}
}
#pragma endregion
