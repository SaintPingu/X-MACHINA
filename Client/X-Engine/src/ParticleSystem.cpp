#include "EnginePch.h"
#include "Component/ParticleSystem.h"
#include "Timer.h"
#include "ResourceMgr.h"
#include "Mesh.h"
#include "DXGIMgr.h"
#include "Shader.h"
#include "FrameResource.h"
#include "Texture.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region ParticleSystem
void ParticleSystem::Init()
{
#pragma region Init_ParticleSystem
	// 파티클 시스템 인덱스를 얻기 위한 용도
	mPSGD = std::make_unique<ParticleSystemGPUData>();
	FRAME_RESOURCE_MGR->CopyData(mPSIdx, *mPSGD.get());
#pragma endregion

#pragma region Init_Particles
	// 파티클 개수 적당히 크게 잡아놓기
	mParticles = std::make_unique<UploadBuffer<ParticleData>>(DEVICE.Get(), 500, false);
#pragma endregion
}

ParticleSystem* ParticleSystem::Play(rsptr<ParticleSystemCPUData> pscd, Transform* target)
{
	mPSCD = pscd;
	ParticleSystemGPUData* psgd = RESOURCE<ParticleSystemGPULoadData>(pscd->mName).get();
	std::memcpy(mPSGD.get(), psgd, sizeof(ParticleSystemGPUData));

	mIsUse = true;
	mIsRunning = false;
	mIsStopCreation = false;
	mLoopingElapsed = 0.f;
	mStopElapsed = 0.f;
	mStartElapsed = 0.f;
	mAccElapsed = 0.f;

	mBurstElapseds.resize(mPSCD->Emission.Bursts.size());
	mBurstRunnings.resize(mPSCD->Emission.Bursts.size());
	for (int i = 0; i < mPSCD->Emission.Bursts.size(); ++i) {
		mBurstElapseds[i] = mPSCD->Duration * mPSCD->SimulationSpeed;
		mBurstRunnings[i] = false;
	}

	// 모든 파티클 초기화
	for (int i = 0; i < mPSCD->MaxParticles; ++i) {
		FRAME_RESOURCE_MGR->CopyData(mPSIdx, *mPSGD);
		mParticles->CopyData(i, ParticleData{});
	}

	mTarget = target;
	mPSGD->WorldPos = target->GetPosition() + mPSCD->Position;

	return this;
}

bool ParticleSystem::Update()
{
	if (!mIsUse) {
		return false;
	}

#pragma region Update_Elapsed
	// 시뮬레이션 속도를 위해 델타 타임에 시뮬레이션 속도 적용
	const float kSimulationDeltaTime = DeltaTime() * mPSCD->SimulationSpeed;

	if (mPSCD->Looping || mPSCD->SimulationSpace == PSSimulationSpace::Local)
		mPSGD->WorldPos = mTarget->GetPosition() + mPSCD->Position;

	mPSGD->DeltaTime = kSimulationDeltaTime;
	mPSGD->TotalTime += kSimulationDeltaTime;
	mAccElapsed += kSimulationDeltaTime;
	mIsRunning = true;

	// 정지 시 업데이트
	if (mIsStopCreation)
		mStopElapsed += kSimulationDeltaTime;

	// 루핑 사용 시 업데이트
	if (mPSCD->Looping)
		mLoopingElapsed += kSimulationDeltaTime;

	// 이미션 사용 시 업데이트
	if (mPSCD->Emission.IsOn) {
		for (auto& elapsed : mBurstElapseds) {
			elapsed += kSimulationDeltaTime;
		}
	}
#pragma endregion

	// StartDelay에 따라 업데이트
	if (mPSGD->TotalTime < 0) {
		mIsRunning = false;
		return false;
	}

	// 정지 경과 시간이 최소 생명 주기를 지났다면 파티클 생성 정지
	if (!mPSCD->Looping && (mPSGD->TotalTime >= mPSCD->Duration) || mIsStopCreation) {
		Stop();
		// 정지 경과 시간이 최대 생명 주기를 지났다면 파티클 삭제
		if (mStopElapsed >= max(mPSGD->StartLifeTime.x, mPSGD->StartLifeTime.y) || mPSCD->Prewarm) {
			mIsUse = false;
			mIsRunning = false;

			return true;
		}
	}

	// 이미션이 켜 있는 경우에만 파티클을 추가한다.
	mPSGD->AddCount = 0;
	const float createInterval = 1.f / mPSCD->Emission.RateOverTime;
	if (mPSCD->Emission.IsOn) {
		if (createInterval < mAccElapsed) {
			mAccElapsed -= createInterval;
			mPSGD->AddCount = mIsStopCreation ? 0 : mPSCD->MaxAddCount;
		}
	}

	// 모든 버스트에 대하여 경과 시간이 지나면 count만큼 추가
	for (int i = 0; i < mPSCD->Emission.Bursts.size(); ++i) {
		if (mBurstElapseds[i] >= mPSCD->Emission.Bursts[i].Time) {
			if (mBurstRunnings[i])
				continue;

			// AddCount에 해당 버스트의 Count 추가
			mPSGD->AddCount += mPSCD->Emission.Bursts[i].Count;
			mBurstRunnings[i] = true;

			if (mPSCD->Looping) {
				mBurstElapseds[i] = 0.f;
				mBurstRunnings[i] = false;
			}
		}
	}

	// 메모리 복사
	FRAME_RESOURCE_MGR->CopyData(mPSIdx, *mPSGD.get());
	return false;
}

void ParticleSystem::Stop()
{
	mIsStopCreation = true;
}

void ParticleSystem::Compute() const
{
	CMD_LIST->SetComputeRootUnorderedAccessView(DXGIMgr::I->GetParticleComputeRootParamIndex(RootParam::ComputeParticle), mParticles->Resource()->GetGPUVirtualAddress());
	CMD_LIST->SetComputeRoot32BitConstants(DXGIMgr::I->GetParticleComputeRootParamIndex(RootParam::ParticleIndex), 1, &mPSIdx, 0);

	CMD_LIST->Dispatch(1, 1, 1);
}

void ParticleSystem::Render() const
{
	CMD_LIST->SetGraphicsRootShaderResourceView(DXGIMgr::I->GetGraphicsRootParamIndex(RootParam::Particle), mParticles->Resource()->GetGPUVirtualAddress());
	RESOURCE<ModelObjectMesh>("Point")->RenderInstanced(mPSCD->MaxParticles);
}

void ParticleSystem::ReturnIndex()
{
	FRAME_RESOURCE_MGR->ReturnIndex(mPSIdx, BufferType::ParticleSystem);
	FRAME_RESOURCE_MGR->ReturnIndex(mPSIdx, BufferType::ParticleShared);
}

void ParticleSystem::SavePSCD(ParticleSystemCPUData& pscd)
{
	std::string filePath = "Import/ParticleSystems/" + pscd.mName + ".xml";

	std::ofstream out(filePath);

	if (out) {
		boost::archive::xml_oarchive oa(out);
		oa << BOOST_SERIALIZATION_NVP(pscd);
	}
}

sptr<ParticleSystemCPUData> ParticleSystem::LoadPSCD(const std::string& filePath)
{
	sptr<ParticleSystemCPUData> pscd = std::make_shared<ParticleSystemCPUData>();

	std::ifstream in(filePath);

	if (in) {
		boost::archive::xml_iarchive ia(in);
		ia >> BOOST_SERIALIZATION_NVP(*pscd.get());
	}

	return pscd;
}

sptr<ParticleSystemGPULoadData> ParticleSystem::LoadPSGD(const std::string& pscdName)
{
	auto& pscd = RESOURCE<ParticleSystemCPUData>(pscdName);
	sptr<ParticleSystemGPULoadData> psgd = std::make_shared<ParticleSystemGPULoadData>();

	psgd->StartLifeTime = pscd->StartLifeTime;
	psgd->StartSpeed = pscd->StartSpeed;
	psgd->MaxParticles = pscd->MaxParticles;
	psgd->StartSize3D = pscd->StartSize3D;
	psgd->StartSize = pscd->StartSize;
	psgd->StartRotation3D = pscd->StartRotation3D;
	psgd->StartRotation = pscd->StartRotation;
	psgd->StartColor = pscd->StartColor;
	psgd->GravityModifier = pscd->GravityModifier;
	psgd->SimulationSpace = pscd->SimulationSpace;
	psgd->SimulationSpeed = pscd->SimulationSpeed;
	psgd->VelocityOverLifetime = pscd->VelocityOverLifetime;
	psgd->ColorOverLifetime = pscd->ColorOverLifetime;
	psgd->SizeOverLifetime = pscd->SizeOverLifetime;
	psgd->RotationOverLifetime = pscd->RotationOverLifetime;
	psgd->Shape = pscd->Shape;
	psgd->TextureIndex = RESOURCE<Texture>(pscd->Renderer.TextureName)->GetSrvIdx();
	psgd->Duration = pscd->Duration;
	psgd->DeltaTime = 0.f;
	psgd->TotalTime = -pscd->StartDelay;

#pragma region RenderMode
	// 렌더 모드가 Stretched일 경우 StartSize3D를 사용하여 y축의 값만 변경
	if (pscd->Renderer.RenderMode == PSRenderMode::StretchedBillboard) {
		// StartSize3D를 사용하지 않는 경우 StartSize 값을 StartSize3D에 저장
		if (pscd->StartSize3D.w == 0.f) {
			psgd->StartSize3D.x = pscd->StartSize.x;
			psgd->StartSize3D.y = pscd->StartSize.x * pscd->Renderer.LengthScale;
		}
		// StartSize3D를 사용할 경우 StartSize3D 값을 그대로 사용
		else {
			psgd->StartSize3D.x = pscd->StartSize3D.x;
			psgd->StartSize3D.y = psgd->StartSize3D.y * pscd->Renderer.LengthScale;
		}
		// StartSize3D를 사용할 것이기 때문에 w 값을 1로 변경
		psgd->StartSize3D.w = 1.f;
	}
#pragma endregion

#pragma region BlendType
	if (pscd->Renderer.BlendType == BlendType::One_To_One_Blend || pscd->Renderer.BlendType == BlendType::One_To_One_Stretched_Blend) {
		psgd->StartColor.Vals[0] *= psgd->StartColor.Vals[0].w;
		psgd->StartColor.Vals[1] *= psgd->StartColor.Vals[1].w;
		psgd->StartColor.Vals[2] *= psgd->StartColor.Vals[2].w;
		psgd->StartColor.Vals[3] *= psgd->StartColor.Vals[3].w;
	}
#pragma endregion

	return psgd;
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region ParticleSystemPool
ParticleSystemPool::ParticleSystemPool()
{
	mFirstAvailable = &mPSs[0];

	for (int i = 0; i < mkPoolSize - 1; ++i) {
		mPSs[i].Init();
		mPSs[i].SetNext(&mPSs[i + 1]);
	}

	mPSs[mkPoolSize - 1].Init();
	mPSs[mkPoolSize - 1].SetNext(nullptr);
}

ParticleSystem* ParticleSystemPool::Create(rsptr<ParticleSystemCPUData> pscd, Transform* target)
{
	assert(mFirstAvailable);

	ParticleSystem* newPS = mFirstAvailable;
	mFirstAvailable = newPS->GetNext();
	mUseCount++;

	return newPS->Play(pscd, target);
}

void ParticleSystemPool::Clear()
{
	for (auto& ps : mPSs) {
		ps.Stop();
		ps.mIsUse = false;
		ps.mIsRunning = false;
	}
}

void ParticleSystemPool::Update()
{
	for (auto& ps : mPSs) {
		if (ps.Update()) {
			ps.SetNext(mFirstAvailable);
			mFirstAvailable = &ps;
			mUseCount--;
		}
	}
}

void ParticleSystemPool::ComputePSs() const
{
	for (auto& ps : mPSs) {
		if (ps.IsUse() && ps.IsRunning()) {
			ps.Compute();
		}
	}
}

void ParticleSystemPool::RenderPSs() const
{
	for (auto& ps : mPSs) {
		if (ps.IsUse() && ps.IsRunning()) {
			ps.Render();
		}
	}
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region ParticleManager
void ParticleManager::Init()
{
	for (auto& pool : mPSPools) {
		pool = std::make_shared<ParticleSystemPool>();
	}

	mCompute = RESOURCE<Shader>("ComputeParticle");
	mShaders[static_cast<UINT8>(BlendType::One_To_One_Blend)] = RESOURCE<Shader>("OneToOneBlend_GraphicsParticle");
	mShaders[static_cast<UINT8>(BlendType::One_To_One_Blend_ScrollAlphaMask)] = RESOURCE<Shader>("OneToOneBlend_GraphicsScrollAlphaMaskParticle");
	mShaders[static_cast<UINT8>(BlendType::One_To_One_Stretched_Blend)] = RESOURCE<Shader>("OneToOneBlend_GraphicsStretchedParticle");
	mShaders[static_cast<UINT8>(BlendType::Additive_Soft_Blend)] = RESOURCE<Shader>("AdditiveSoft_GraphicsParticle");
	mShaders[static_cast<UINT8>(BlendType::Additive_Soft_Stretched_Blend)] = RESOURCE<Shader>("AdditiveSoft_GraphicsStretchedParticle");
	mShaders[static_cast<UINT8>(BlendType::Multiply_Blend)] = RESOURCE<Shader>("MultiplyBlend_GraphicsParticle");
	mShaders[static_cast<UINT8>(BlendType::Multiply_Stretched_Blend)] = RESOURCE<Shader>("MultiplyBlend_GraphicsStretchedParticle");
	mShaders[static_cast<UINT8>(BlendType::Multiply_Blend_ScrollAlphaMask)] = RESOURCE<Shader>("MultiplyBlend_GraphicsScrollAlphaMaskParticle");
	mShaders[static_cast<UINT8>(BlendType::Alpha_Blend)] = RESOURCE<Shader>("GraphicsParticle");
	mShaders[static_cast<UINT8>(BlendType::Alpha_Stretched_Blend)] = RESOURCE<Shader>("GraphicsStretchedParticle");
	mShaders[static_cast<UINT8>(BlendType::Scroll_Smoke)] = RESOURCE<Shader>("Scroll_Smoke");
}

void ParticleManager::Clear()
{
	for (auto& pool : mPSPools) {
		pool->Clear();
	}
}

ParticleSystem* ParticleManager::Play(const std::string& pscdName, Transform* target)
{
#ifdef RENDER_FOR_SERVER
	return nullptr;
#endif

	const auto& pscd = RESOURCE<ParticleSystemCPUData>(pscdName);
	return mPSPools[static_cast<UINT8>(pscd->Renderer.BlendType)]->Create(pscd, target);
}

void ParticleManager::Update()
{
	for (auto& pool : mPSPools) {
		pool->Update();
	}
}

void ParticleManager::Render() const
{
	// 컴퓨트 쉐이더 실행
	mCompute->Set();
	for (auto& pool : mPSPools) 
	{
		if (!pool->IsUse()) {
			continue;
		}

		pool->ComputePSs();
	}

	for (int type = 0; type < BlendTypeCount; ++type) {
		if (!mPSPools[type]->IsUse()) {
			continue;
		}

		if (mShaders[type]) {
			mShaders[type]->Set();
		}

		mPSPools[type]->RenderPSs();
	}
}
#pragma endregion

