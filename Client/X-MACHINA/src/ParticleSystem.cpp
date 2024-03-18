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
	mPSGD.SizeOverLifeTime = mPSCD.SizeOverLifeTime;
	mPSGD.ColorOverLifeTime = mColorOverLifeTime;

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
	mEmission.UpdateDeltaTime(kSimulationDeltaTime);

	mPSGD.AddCount = 0;
	const float createInterval = mEmission.IsOn ? (1.f / mEmission.RateOverTime) : 0.f;
	if (createInterval < mPSCD.AccTime) {
		mPSCD.AccTime = mPSCD.AccTime - (1.f / mEmission.RateOverTime);
		mPSGD.AddCount = mPSCD.IsStop ? 0 : mPSCD.MaxAddCount;
	}

	// 모든 버스트에 대하여 경과 시간이 지나면 count만큼 추가
	for (auto& burst : mEmission.Bursts) {
		if (burst.BurstElapsed >= mPSCD.Duration) {
			mPSGD.AddCount += burst.Count;
			burst.BurstElapsed = 0.f;
		}
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
	mAlphaShader = res->Get<Shader>("GraphicsParticle");
	mOneToOneShader = res->Get<Shader>("OneToOneBlend_GraphicsParticle");
	mAlphaStretchedShader = res->Get<Shader>("GraphicsStretchedParticle");
	mOneToOneStretchedShader = res->Get<Shader>("OneToOneBlend_GraphicsStretchedParticle");
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
	BlendType prevBlendType = BlendType::Alpha_Blend;
	for (const auto& ps : mParticleSystems) {
		const PSRenderMode currMode = ps.second->GetRenderer().RenderMode;
		const BlendType currBlendType = ps.second->GetRenderer().BlendType;

		if (prevMode != currMode || prevBlendType != currBlendType) {
			switch (currMode)
			{
			case PSRenderMode::Billboard:
				if (currBlendType == BlendType::Alpha_Blend)
					mAlphaShader->Set();
				else if (currBlendType == BlendType::One_To_One_Blend)
					mOneToOneShader->Set();
				break;
			case PSRenderMode::StretchedBillboard:
				if (currBlendType == BlendType::Alpha_Blend)
					mAlphaStretchedShader->Set();
				else if (currBlendType == BlendType::One_To_One_Blend)
					mOneToOneStretchedShader->Set();
				break;
			}
			prevMode = currMode;
			prevBlendType = currBlendType;
		}

		ps.second->RenderParticleSystem();
	}
}
#pragma endregion
