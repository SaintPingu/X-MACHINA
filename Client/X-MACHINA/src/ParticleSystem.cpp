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
	mParticles = std::make_unique<UploadBuffer<ParticleData>>(device.Get(), mPSCD.MaxParticles, false);
#pragma endregion

	if (mPSCD.PlayOnAwake) {
		Play();
	}
}

void ParticleSystem::Update()
{
	mPSGD.StartLifeTime = mPSCD.StartLifeTime;
	mPSGD.StartSpeed = mPSCD.StartSpeed;
	mPSGD.MaxParticles = mPSCD.MaxParticles;
	mPSGD.TextureIndex = mPSCD.TextureIndex;
	mPSGD.StartSize3D = mPSCD.StartSize3D;
	mPSGD.StartSize = mPSCD.StartSize;
	mPSGD.StartRotation3D = mPSCD.StartRotation3D;
	mPSGD.StartRotation = mPSCD.StartRotation;
	mPSGD.StartColor = mPSCD.StartColor;
	mPSGD.GravityModifier = mPSCD.GravityModifier;
	mPSGD.SimulationSpace = mPSCD.SimulationSpace;
	mPSGD.SimulationSpeed = mPSCD.SimulationSpeed;

	if (mRenderer.RenderMode == PSRenderMode::StretchedBillboard) {
		if (mPSCD.StartSize3D.w == 0.f) {
			mPSGD.StartSize3D.x = mPSCD.StartSize.x;
			mPSGD.StartSize3D.y = mPSCD.StartSize.x * mRenderer.LengthScale;
			mPSGD.StartSize3D.w = 1.f;
		}
		else {
			mPSGD.StartSize3D.x = mPSCD.StartSize3D.x;
			mPSGD.StartSize3D.y = mPSGD.StartSize3D.y * mRenderer.LengthScale;
			mPSGD.StartSize3D.w = 1.f;
		}
	}

	const float kSimulationDeltaTime = DeltaTime() * mPSCD.SimulationSpeed;

#pragma region Check_Looping
	if (!mPSCD.Looping && mPSCD.LoopingElapsed >= mPSCD.Duration) {
		Stop();
	}
#pragma endregion

#pragma region Check_StartDelay
	mPSCD.StartElapsed += kSimulationDeltaTime;
	if (mPSCD.StartElapsed <= mPSCD.StartDelay) {
		return;
	}
#pragma endregion

#pragma region Check_StopDelay
	if (mPSCD.IsStop) {
		mPSCD.StopElapsed += kSimulationDeltaTime;
		if (mPSCD.StopElapsed >= (!mPSCD.Prewarm ? mPSGD.StartLifeTime.y : 0)) {
			SetActive(false);

			pr->RemoveParticleSystem(mPSIdx);

			if (mIsDeprecated)
				ReturnIndex();

			return;
		}
	}
#pragma endregion
#pragma region Update
	mPSGD.WorldPos = mTarget->GetPosition();
	mPSGD.DeltaTime = kSimulationDeltaTime;
	mPSCD.LoopingElapsed += kSimulationDeltaTime;
	mPSCD.AccTime += kSimulationDeltaTime;
	mPSGD.TotalTime += kSimulationDeltaTime;

	mPSGD.AddCount = 0;
	if ((1.f / mPSCD.RateOverTime) < mPSCD.AccTime) {
		mPSCD.AccTime = mPSCD.AccTime - (1.f / mPSCD.RateOverTime);
		mPSGD.AddCount = mPSCD.IsStop ? 0 : mPSCD.MaxAddCount;
	}

	frmResMgr->CopyData(mPSIdx, mPSGD);
#pragma endregion
}

void ParticleSystem::OnDestroy()
{
	if (mIsDeprecated)
		return;

	Stop();
	mIsDeprecated = true;
	pr->AddParticleSystem(shared_from_this());
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
	res->Get<ModelObjectMesh>("Point")->RenderInstanced(mPSCD.MaxParticles);
}

void ParticleSystem::ReturnIndex()
{
	frmResMgr->ReturnIndex(mPSIdx, BufferType::ParticleSystem);
	frmResMgr->ReturnIndex(mPSIdx, BufferType::ParticleShared);
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region ParticleRenderer
void ParticleRenderer::Init()
{
	mParticleSystems.reserve(300);
	mDeprecations.reserve(300);
	mComputeShader = res->Get<Shader>("ComputeParticle");
	mBillboardShader = res->Get<Shader>("GraphicsParticle");
	mStretchedBillboardShader = res->Get<Shader>("GraphicsStretchedParticle");
}

void ParticleRenderer::AddParticleSystem(sptr<ParticleSystem> particleSystem)
{
	mParticleSystems.insert(std::make_pair(particleSystem->GetPSIdx(), particleSystem));

	if (particleSystem->IsDeprecated()) {
		mDeprecations.insert(std::make_pair(particleSystem->GetPSIdx(), particleSystem));
	}
}

void ParticleRenderer::RemoveParticleSystem(int particleSystemIdx)
{
	mRemoval.push(particleSystemIdx);
}

void ParticleRenderer::Update()
{
	for (const auto& ps : mDeprecations) {
		ps.second->Update();
	}

	while (!mRemoval.empty()) {
		int deprecated = mRemoval.front();
		mDeprecations.erase(deprecated);
		mParticleSystems.erase(deprecated);
		mRemoval.pop();
	}
}

void ParticleRenderer::Render() const
{
	mComputeShader->Set();
	for (const auto& ps : mParticleSystems) {
		ps.second->ComputeParticleSystem();
	}

	PSRenderMode prevMode = PSRenderMode::None;
	for (const auto& ps : mParticleSystems) {
		const PSRenderMode currMode = ps.second->GetRenderer().RenderMode;

		if (prevMode != currMode) {
			switch (currMode)
			{
			case PSRenderMode::Billboard:
				mBillboardShader->Set();
				break;
			case PSRenderMode::StretchedBillboard:
				mStretchedBillboardShader->Set();
				break;
			}
			prevMode = currMode;
		}

		ps.second->RenderParticleSystem();
	}
}
#pragma endregion
