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
sptr<ParticleSystem> ParticleSystem::SetTarget(const std::string& frameName)
{
	// 파티클 시스템을 부착시킬 타겟을 설정
	Transform* findFrame = mObject->FindFrame(frameName);
	if (findFrame) {
		mTarget = findFrame;
	}

	return shared_from_this();
}

void ParticleSystem::SetSizeByRenderMode(PSRenderMode renderMode)
{
	// 렌더 모드가 Stretched일 경우 StartSize3D를 사용하여 y축의 값만 변경
	if (renderMode == PSRenderMode::StretchedBillboard) {
		// StartSize3D를 사용하지 않는 경우 StartSize 값을 StartSize3D에 저장
		if (mPSCD->StartSize3D.w == 0.f) {
			mPSGD.StartSize3D.x = mPSCD->StartSize.x;
			mPSGD.StartSize3D.y = mPSCD->StartSize.x * mPSCD->Renderer.LengthScale;
		}
		// StartSize3D를 사용할 경우 StartSize3D 값을 그대로 사용
		else {
			mPSGD.StartSize3D.x = mPSCD->StartSize3D.x;
			mPSGD.StartSize3D.y = mPSGD.StartSize3D.y * mPSCD->Renderer.LengthScale;
		}
		// StartSize3D를 사용할 것이기 때문에 w 값을 1로 변경
		mPSGD.StartSize3D.w = 1.f;
	}
}

void ParticleSystem::SetColorByBlendType(BlendType blendType)
{
	if (blendType == BlendType::One_To_One_Blend || blendType == BlendType::One_To_One_Stretched_Blend) {
		mPSGD.StartColor.FirstColor *= mPSGD.StartColor.FirstColor.w;
		mPSGD.StartColor.FirstGradient *= mPSGD.StartColor.FirstGradient.w;
		mPSGD.StartColor.SecondColor *= mPSGD.StartColor.SecondColor.w;
		mPSGD.StartColor.SecondGradient *= mPSGD.StartColor.SecondGradient.w;
	}
}

void ParticleSystem::Awake()
{
	base::Awake();

#pragma region Init_ParticleSystem
	// 파티클 시스템 인덱스를 얻기 위한 용도
	FRAME_RESOURCE_MGR->CopyData(mPSIdx, mPSGD);
#pragma endregion

#pragma region Init_Particles
	// 파티클 최대 개수만큼 생성
	mParticles = std::make_unique<UploadBuffer<ParticleData>>(DEVICE.Get(), mPSCD->MaxParticles, false);
#pragma endregion

#pragma region Init_PSGD
	// 응용 프로그램에서 설정한 값(PSCD)을 GPU에 넘겨주기 위하여 PSGD 업데이트 
	mPSGD.StartLifeTime			= mPSCD->StartLifeTime;
	mPSGD.StartSpeed			= mPSCD->StartSpeed;
	mPSGD.MaxParticles			= mPSCD->MaxParticles;
	mPSGD.StartSize3D			= mPSCD->StartSize3D;
	mPSGD.StartSize				= mPSCD->StartSize;
	mPSGD.StartRotation3D		= mPSCD->StartRotation3D;
	mPSGD.StartRotation			= mPSCD->StartRotation;
	mPSGD.StartColor			= mPSCD->StartColor;
	mPSGD.GravityModifier		= mPSCD->GravityModifier;
	mPSGD.SimulationSpace		= mPSCD->SimulationSpace;
	mPSGD.SimulationSpeed		= mPSCD->SimulationSpeed;
	mPSGD.VelocityOverLifetime	= mPSCD->VelocityOverLifetime;
	mPSGD.ColorOverLifetime		= mPSCD->ColorOverLifetime;
	mPSGD.SizeOverLifetime		= mPSCD->SizeOverLifetime;
	mPSGD.RotationOverLifetime	= mPSCD->RotationOverLifetime;
	mPSGD.Shape					= mPSCD->Shape;
	mPSGD.TextureIndex			= RESOURCE<Texture>(mPSCD->Renderer.TextureName)->GetSrvIdx();

	SetSizeByRenderMode(mPSCD->Renderer.RenderMode);
	SetColorByBlendType(mPSCD->Renderer.BlendType);

	mBurstElapseds.resize(mPSCD->Emission.Bursts.size());
	mBurstRunnings.resize(mPSCD->Emission.Bursts.size());
#pragma endregion

	// 최초 실행되자마자 플레이
	if (mPSCD->PlayOnAwake)
		Play();
}

void ParticleSystem::UpdateParticleSystem()
{
#pragma region Update_Elapsed
	// 시뮬레이션 속도를 위해 델타 타임에 시뮬레이션 속도 적용
	const float kSimulationDeltaTime = DeltaTime() * mPSCD->SimulationSpeed;

	// 타겟 위치 저장 및 시뮬레이션 델타 타임 적용
	mPSGD.WorldPos = mTarget->GetPosition();
	mPSGD.DeltaTime = kSimulationDeltaTime;
	mPSGD.TotalTime += kSimulationDeltaTime;
	mAccElapsed += kSimulationDeltaTime;

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
	if (mPSGD.TotalTime < mPSCD->StartDelay) {
		mIsRunning = false;
		return;
	}

	// 정지 경과 시간이 최소 생명 주기를 지났다면 파티클 생성 정지
	if (!mPSCD->Looping && (mPSGD.TotalTime - mPSCD->StartDelay) >= mPSCD->Duration || mIsStopCreation) {
		Stop();
		// 정지 경과 시간이 최대 생명 주기를 지났다면 파티클 삭제
		if (mStopElapsed >= max(mPSGD.StartLifeTime.x, mPSGD.StartLifeTime.y)) {
			Reset();
			mIsRunning = false;
			ParticleRenderer::I->RemoveParticleSystem(mPSCD->Renderer.BlendType, mPSIdx);
			return;
		}
	}

	// 이미션이 켜 있는 경우에만 파티클을 추가한다.
	mPSGD.AddCount = 0;
	const float createInterval = 1.f / mPSCD->Emission.RateOverTime;
	if (mPSCD->Emission.IsOn) {
		if (createInterval < mAccElapsed) {
			mAccElapsed -= createInterval;
			mPSGD.AddCount = mIsStopCreation ? 0 : mPSCD->MaxAddCount;
		}
	}

	// 모든 버스트에 대하여 경과 시간이 지나면 count만큼 추가
	for (int i = 0; i < mPSCD->Emission.Bursts.size(); ++i) {
		if (mBurstElapseds[i] >= mPSCD->Emission.Bursts[i].Time) {
			if (mBurstRunnings[i])
				continue;

			// AddCount에 해당 버스트의 Count 추가
			mPSGD.AddCount += mPSCD->Emission.Bursts[i].Count;
			mBurstRunnings[i] = true;

			if (mPSCD->Looping) {
				mBurstElapseds[i] = 0.f;
				mBurstRunnings[i] = false;
			}
		}
	}

	// 메모리 복사
	FRAME_RESOURCE_MGR->CopyData(mPSIdx, mPSGD);

	mIsRunning = true;
}

void ParticleSystem::Play()
{
	// 플레이 시 파티클 렌더러에 추가 후 경과 시간 초기화
	ParticleRenderer::I->AddParticleSystem(mPSCD->Renderer.BlendType, shared_from_this());
	mIsStopCreation		= false;
	mLoopingElapsed		= 0.f;
	mStopElapsed		= 0.f;
	mStartElapsed		= 0.f;
	mAccElapsed			= 0.f;
	mPSGD.TotalTime		= 0.f;
	mPSGD.DeltaTime		= 0.f;

	// 처음 시작하자마자 파티클을 생성하기 위함
	for (int i = 0; i < mPSCD->Emission.Bursts.size(); ++i) {
		mBurstElapseds[i] = mPSCD->Duration * mPSCD->SimulationSpeed;
		mBurstRunnings[i] = false;
	}
}

void ParticleSystem::Stop()
{
	mIsStopCreation = true;
}

void ParticleSystem::Reset()
{
	for (int i = 0; i < mPSCD->MaxParticles; ++i) {
		FRAME_RESOURCE_MGR->CopyData(mPSIdx, mPSGD);
		mParticles->CopyData(i, ParticleData{});
	}
}

void ParticleSystem::ComputeParticleSystem() const
{
	CMD_LIST->SetComputeRootUnorderedAccessView(DXGIMgr::I->GetParticleComputeRootParamIndex(RootParam::ComputeParticle), mParticles->Resource()->GetGPUVirtualAddress());
	CMD_LIST->SetComputeRoot32BitConstants(DXGIMgr::I->GetParticleComputeRootParamIndex(RootParam::ParticleIndex), 1, &mPSIdx, 0);

	CMD_LIST->Dispatch(1, 1, 1);
}

void ParticleSystem::RenderParticleSystem() const
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

sptr<ParticleSystem> ParticleSystem::Load(const std::string& fileName)
{
	mPSCD = RESOURCE<ParticleSystemCPUData>(fileName);
	return shared_from_this();
}

sptr<ParticleSystemCPUData> ParticleSystem::LoadPSCD(const std::string& filePath)
{
	sptr<ParticleSystemCPUData> pscd = std::make_shared< ParticleSystemCPUData>();

	std::ifstream in(filePath);

	if (in) {
		boost::archive::xml_iarchive ia(in);
		ia >> BOOST_SERIALIZATION_NVP(*pscd.get());
	}

	return pscd;
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region ParticleRenderer
void ParticleRenderer::Init()
{
	for (auto& ps : mPSs) {
		ps.reserve(10000);
	}

	mCompute = RESOURCE<Shader>("ComputeParticle");
	mShaders[static_cast<UINT8>(BlendType::Alpha_Blend)] = RESOURCE<Shader>("GraphicsParticle");
	mShaders[static_cast<UINT8>(BlendType::Alpha_Stretched_Blend)] = RESOURCE<Shader>("GraphicsStretchedParticle");
	mShaders[static_cast<UINT8>(BlendType::One_To_One_Blend)] = RESOURCE<Shader>("OneToOneBlend_GraphicsParticle");
	mShaders[static_cast<UINT8>(BlendType::One_To_One_Blend_ScrollAlphaMask)] = RESOURCE<Shader>("OneToOneBlend_GraphicsScrollAlphaMaskParticle");
	mShaders[static_cast<UINT8>(BlendType::One_To_One_Stretched_Blend)] = RESOURCE<Shader>("OneToOneBlend_GraphicsStretchedParticle");
	mShaders[static_cast<UINT8>(BlendType::Additive_Soft_Blend)] = RESOURCE<Shader>("AdditiveSoft_GraphicsParticle");
	mShaders[static_cast<UINT8>(BlendType::Additive_Soft_Stretched_Blend)] = RESOURCE<Shader>("AdditiveSoft_GraphicsStretchedParticle");
	mShaders[static_cast<UINT8>(BlendType::Multiply_Blend)] = RESOURCE<Shader>("MultiplyBlend_GraphicsParticle");
	mShaders[static_cast<UINT8>(BlendType::Multiply_Stretched_Blend)] = RESOURCE<Shader>("MultiplyBlend_GraphicsStretchedParticle");
	mShaders[static_cast<UINT8>(BlendType::Multiply_Blend_ScrollAlphaMask)] = RESOURCE<Shader>("MultiplyBlend_GraphicsScrollAlphaMaskParticle");
}

void ParticleRenderer::AddParticleSystem(BlendType type, sptr<ParticleSystem> particleSystem)
{
	mPSs[static_cast<UINT8>(type)].insert(std::make_pair(particleSystem->GetPSIdx(), particleSystem));
}

void ParticleRenderer::RemoveParticleSystem(BlendType type, int particleSystemIdx)
{
	// 최종 삭제 파티클 시스템 인덱스
	mRemovals[static_cast<UINT8>(type)].push(particleSystemIdx);
}

void ParticleRenderer::Update()
{
	for (int type = 0; type < BlendTypeCount; ++type) {
		// 파티클 삭제
		while (!mRemovals[type].empty()) {
			mPSs[type].erase(mRemovals[type].front());
			mRemovals[type].pop();
		}

		// 파티클 시스템 업데이트
		for (const auto& ps : mPSs[type]) {
			ps.second->UpdateParticleSystem();
		}
	}
}

void ParticleRenderer::Render() const
{
	// 컴퓨트 쉐이더 실행
	mCompute->Set();

	for (const auto& pss : mPSs) {
		if (pss.empty())
			continue;

		for (const auto& ps : pss) {
			if (!ps.second->IsRunning())
				continue;

			ps.second->ComputeParticleSystem();
		}
	}

	// 그래픽스 쉐이더 실행
	for (int type = 0; type < BlendTypeCount; ++type) {
		const auto& pss = mPSs[type];
		const auto& shader = mShaders[type];

		if (pss.empty())
			continue;

		shader->Set();

		for (const auto& ps : pss) {
			if (!ps.second->IsRunning())
				continue;

			ps.second->RenderParticleSystem();
		}
	}
}
#pragma endregion
