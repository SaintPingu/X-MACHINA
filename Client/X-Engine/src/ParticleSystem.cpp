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
void ParticleSystem::SetTarget(const std::string& frameName)
{
	// 파티클 시스템을 부착시킬 타겟을 설정
	Transform* findFrame = mObject->FindFrame(frameName);
	if (findFrame) {
		mTarget = findFrame;
	}
}


void ParticleSystem::Awake()
{
	base::Awake();

#pragma region Init_ParticleSystem
	// 파티클 시스템 인덱스를 얻기 위한 용도
	frmResMgr->CopyData(mPSIdx, mPSGD);
#pragma endregion

#pragma region Init_Particles
	// 파티클 최대 개수만큼 생성
	mParticles = std::make_unique<UploadBuffer<ParticleData>>(device.Get(), mPSCD.MaxParticles, false);
#pragma endregion

	// 최초 실행되자마자 플레이
	if (mPSCD.PlayOnAwake) {
		Play();
	}
}


void ParticleSystem::Update()
{
	// 응용 프로그램에서 설정한 값(PSCD)을 GPU에 넘겨주기 위하여 PSGD 업데이트 
	mPSGD.StartLifeTime = mPSCD.StartLifeTime;
	mPSGD.StartSpeed = mPSCD.StartSpeed;
	mPSGD.MaxParticles = mPSCD.MaxParticles;
	mPSGD.StartSize3D = mPSCD.StartSize3D;
	mPSGD.StartSize = mPSCD.StartSize;
	mPSGD.StartRotation3D = mPSCD.StartRotation3D;
	mPSGD.StartRotation = mPSCD.StartRotation;
	mPSGD.StartColor = mPSCD.StartColor;
	mPSGD.GravityModifier = mPSCD.GravityModifier;
	mPSGD.SimulationSpace = mPSCD.SimulationSpace;
	mPSGD.SimulationSpeed = mPSCD.SimulationSpeed;
	mPSGD.SizeOverLifeTime = mPSCD.SizeOverLifeTime;
	mPSGD.ColorOverLifeTime = mPSCD.ColorOverLifeTime;
	mPSGD.Shape = mPSCD.Shape;
	mPSGD.TextureIndex = res->Get<Texture>(mPSCD.Renderer.TextureName)->GetSrvIdx();

	// 렌더 모드가 Stretched일 경우 StartSize3D를 사용하여 y축의 값만 변경
	if (mPSCD.Renderer.RenderMode == PSRenderMode::StretchedBillboard) {
		// StartSize3D를 사용하지 않는 경우 StartSize 값을 StartSize3D에 저장
		if (mPSCD.StartSize3D.w == 0.f) {
			mPSGD.StartSize3D.x = mPSCD.StartSize.x;
			mPSGD.StartSize3D.y = mPSCD.StartSize.x * mPSCD.Renderer.LengthScale;
		}
		// StartSize3D를 사용할 경우 StartSize3D 값을 그대로 사용
		else {
			mPSGD.StartSize3D.x = mPSCD.StartSize3D.x;
			mPSGD.StartSize3D.y = mPSGD.StartSize3D.y * mPSCD.Renderer.LengthScale;
		}
		// StartSize3D를 사용할 것이기 때문에 w 값을 1로 변경
		mPSGD.StartSize3D.w = 1.f;
	}

	// 시뮬레이션 속도를 위해 델타 타임에 시뮬레이션 속도 적용
	const float kSimulationDeltaTime = DeltaTime() * mPSCD.SimulationSpeed;

#pragma region Check_Looping
	// looping을 사용하지 않을 경우 Duration 시간 동안만 파티클 업데이트
	if (!mPSCD.Looping && mPSCD.LoopingElapsed >= mPSCD.Duration) {
		Stop();
	}
#pragma endregion

#pragma region Check_StartDelay
	// StartDelay 이후부터 실행
	mPSCD.StartElapsed += kSimulationDeltaTime;
	if (mPSCD.StartElapsed <= mPSCD.StartDelay) {
		return;
	}
#pragma endregion

#pragma region Check_StopDelay
	// 파티클 시스템 정지가 실행된 경우
	if (mPSCD.IsStop) {
		mPSCD.StopElapsed += kSimulationDeltaTime;
		// Prewarm 값에 따라 true이면 바로 정지, false이면 생명 주기 최대 시간을 기다린 후 정지
		if (mPSCD.StopElapsed >= (!mPSCD.Prewarm ? mPSGD.StartLifeTime.y : 0)) {
			SetActive(false);

			pr->RemoveParticleSystem(mPSIdx);

			// 객체 소멸 이후 파티클도 소멸해야 하는 경우 최종으로 인덱스 반환
			//if (mIsDeprecated)
			//	ReturnIndex();

			return;
		}
	}
#pragma endregion

#pragma region Update
	// 타겟 위치 저장 및 시뮬레이션 델타 타임 적용
	mPSGD.WorldPos = mTarget->GetPosition();
	mPSGD.TotalTime += kSimulationDeltaTime;
	mPSGD.DeltaTime = kSimulationDeltaTime;
	mPSCD.LoopingElapsed += kSimulationDeltaTime;
	mPSCD.AccTime += kSimulationDeltaTime;
	mPSCD.Emission.UpdateDeltaTime(kSimulationDeltaTime);

	// 파티클 생성 시간이 되면 AddCount 증가
	mPSGD.AddCount = 0;
	const float createInterval = mPSCD.Emission.IsOn ? (1.f / mPSCD.Emission.RateOverTime) : 0.f;
	if (createInterval < mPSCD.AccTime) {
		// 누적 시간 초기화 및 AddCount 변경
		mPSCD.AccTime = mPSCD.AccTime - (1.f / mPSCD.Emission.RateOverTime);
		mPSGD.AddCount = mPSCD.IsStop ? 0 : mPSCD.MaxAddCount;
	}

	// 모든 버스트에 대하여 경과 시간이 지나면 count만큼 추가
	for (auto& burst : mPSCD.Emission.Bursts) {
		if (burst.BurstElapsed >= mPSCD.Duration) {
			// AddCount에 해당 버스트의 Count 추가
			mPSGD.AddCount += burst.Count;
			burst.BurstElapsed = 0.f;
		}
	}

	// 메모리 복사
	frmResMgr->CopyData(mPSIdx, mPSGD);
#pragma endregion
}

void ParticleSystem::OnDestroy()
{
	// 파티클 시스템을 물고 있는 객체 소멸 시 호출
	if (mIsDeprecated)
		return;

	// 물고 있는 객체가 소멸되더라도 바로 소멸되지 않도록 파티클 렌더러에 소유권 이전
	//Stop();
	mIsDeprecated = true;
	pr->AddParticleSystem(shared_from_this());
}

void ParticleSystem::Play()
{
	// 플레이 시 파티클 렌더러에 추가 후 경과 시간 초기화
	if (mPSCD.IsStop) {
		SetActive(true);
		pr->AddParticleSystem(shared_from_this());
		mPSCD.LoopingElapsed = 0.f;
		mPSCD.StopElapsed = 0.f;
		mPSCD.StartElapsed = 0.f;

		mPSGD.TotalTime = 0.f;
		mPSGD.DeltaTime = 0.f;
		mPSCD.AccTime += 0.f;

		mPSCD.IsStop = false;
		mIsDeprecated = false;
	}
}

void ParticleSystem::Stop()
{
	// Stop 플래그 설정, 정지 플래그라기 보다는 정지 알림 플래그
	if (!mPSCD.IsStop) {
		mPSCD.IsStop = true;
	}
}

void ParticleSystem::PlayToggle()
{
	// 플레이 토글
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

void ParticleSystem::Save()
{
	std::string filePath = "Import/ParticleSystem/" + mPSCD.mName + ".xml";

	std::ofstream out(filePath);

	if (out) {
		boost::archive::xml_oarchive oa(out);
		oa << BOOST_SERIALIZATION_NVP(mPSCD);
	}
}

sptr<ParticleSystem> ParticleSystem::Load(const std::string& fileName)
{
	std::string filePath = "Import/ParticleSystem/" + fileName + ".xml";

	std::ifstream in(filePath);

	if (in) {
		boost::archive::xml_iarchive ia(in);
		ia >> BOOST_SERIALIZATION_NVP(mPSCD);
	}

	return shared_from_this();
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
	printf("Add : %d\n", particleSystem->GetPSIdx());
	mParticleSystems.insert(std::make_pair(particleSystem->GetPSIdx(), particleSystem));

	// 파티클 시스템이 삭제 예정 파티클이라면 Deprecations 컨테이너에도 추가
	if (particleSystem->IsDeprecated()) {
		mDeprecations.insert(std::make_pair(particleSystem->GetPSIdx(), particleSystem));
	}
}

void ParticleRenderer::RemoveParticleSystem(int particleSystemIdx)
{
	// 최종 삭제 파티클 시스템 인덱스
	mRemoval.push(particleSystemIdx);
}

void ParticleRenderer::Update()
{
	// 파티클 시스템을 물고 있는 객체가 삭제 되더라도 바로 소멸되지 않도록 업데이트
	for (const auto& ps : mDeprecations) {
		ps.second->Update();
	}

	// 파티클 삭제
	while (!mRemoval.empty()) {
		int deprecated = mRemoval.front();
		mDeprecations.erase(deprecated);
		mParticleSystems.erase(deprecated);
		mRemoval.pop();
	}
}

void ParticleRenderer::Render() const
{
	// 컴퓨트 쉐이더 실행
	mComputeShader->Set();
	for (const auto& ps : mParticleSystems) {
		ps.second->ComputeParticleSystem();
	}

	// 쉐이더가 같은 경우 쉐이더를 set 하지 않는다.
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
