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
	// ��ƼŬ �ý����� ������ų Ÿ���� ����
	Transform* findFrame = mObject->FindFrame(frameName);
	if (findFrame) {
		mTarget = findFrame;
	}
}

void ParticleSystem::SetSizeByRenderMode(PSRenderMode renderMode)
{
	// ���� ��尡 Stretched�� ��� StartSize3D�� ����Ͽ� y���� ���� ����
	if (renderMode == PSRenderMode::StretchedBillboard) {
		// StartSize3D�� ������� �ʴ� ��� StartSize ���� StartSize3D�� ����
		if (mPSCD->StartSize3D.w == 0.f) {
			mPSGD.StartSize3D.x = mPSCD->StartSize.x;
			mPSGD.StartSize3D.y = mPSCD->StartSize.x * mPSCD->Renderer.LengthScale;
		}
		// StartSize3D�� ����� ��� StartSize3D ���� �״�� ���
		else {
			mPSGD.StartSize3D.x = mPSCD->StartSize3D.x;
			mPSGD.StartSize3D.y = mPSGD.StartSize3D.y * mPSCD->Renderer.LengthScale;
		}
		// StartSize3D�� ����� ���̱� ������ w ���� 1�� ����
		mPSGD.StartSize3D.w = 1.f;
	}
}

void ParticleSystem::Awake()
{
	base::Awake();

#pragma region Init_ParticleSystem
	// ��ƼŬ �ý��� �ε����� ��� ���� �뵵
	FRAME_RESOURCE_MGR->CopyData(mPSIdx, mPSGD);
#pragma endregion

#pragma region Init_Particles
	// ��ƼŬ �ִ� ������ŭ ����
	mParticles = std::make_unique<UploadBuffer<ParticleData>>(DEVICE.Get(), mPSCD->MaxParticles, false);
#pragma endregion

#pragma region Init_PSGD
	// ���� ���α׷����� ������ ��(PSCD)�� GPU�� �Ѱ��ֱ� ���Ͽ� PSGD ������Ʈ 
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
	mPSGD.SizeOverLifeTime		= mPSCD->SizeOverLifeTime;
	mPSGD.ColorOverLifeTime		= mPSCD->ColorOverLifeTime;
	mPSGD.Shape					= mPSCD->Shape;
	mPSGD.TextureIndex			= RESOURCE<Texture>(mPSCD->Renderer.TextureName)->GetSrvIdx();

	SetSizeByRenderMode(mPSCD->Renderer.RenderMode);
	mBurstElapseds.resize(mPSCD->Emission.Bursts.size());
	mBurstRunnings.resize(mPSCD->Emission.Bursts.size());
#pragma endregion

	// ���� ������ڸ��� �÷���
	if (mPSCD->PlayOnAwake) {
		Play();
	}
}

void ParticleSystem::Update()
{
#pragma region Update_Elapsed
	// �ùķ��̼� �ӵ��� ���� ��Ÿ Ÿ�ӿ� �ùķ��̼� �ӵ� ����
	const float kSimulationDeltaTime = DeltaTime() * mPSCD->SimulationSpeed;

	// Ÿ�� ��ġ ���� �� �ùķ��̼� ��Ÿ Ÿ�� ����
	mPSGD.WorldPos = mTarget->GetPosition();
	mPSGD.DeltaTime = kSimulationDeltaTime;
	mPSGD.TotalTime += kSimulationDeltaTime;
	mAccElapsed += kSimulationDeltaTime;

	// ���� �� ������Ʈ
	if (mIsStopCreation)
		mStopElapsed += kSimulationDeltaTime;

	// ���� ��� �� ������Ʈ
	if (mPSCD->Looping)
		mLoopingElapsed += kSimulationDeltaTime;

	// �̹̼� ��� �� ������Ʈ
	if (mPSCD->Emission.IsOn) {
		for (auto& elapsed : mBurstElapseds) {
			elapsed += kSimulationDeltaTime;
		}
	}
#pragma endregion

	// StartDelay�� ���� ������Ʈ
	if (mPSGD.TotalTime < mPSCD->StartDelay) {
		mIsRunning = false;
		return;
	}

	mPSGD.AddCount = 0;

	// ���� ��� �ð��� �ּ� ���� �ֱ⸦ �����ٸ� ��ƼŬ ���� ����
	if (!mPSCD->Looping && (mPSGD.TotalTime - mPSCD->StartDelay) >= mPSGD.StartLifeTime.x) {
		Stop();
		if (mPSGD.TotalTime >= mPSCD->PlayMaxTime) {
			// ��� ��ƼŬ �ʱ�ȭ
			for (int i = 0; i < mPSCD->MaxParticles; ++i) {
				FRAME_RESOURCE_MGR->CopyData(mPSIdx, mPSGD);
				mParticles->CopyData(i, ParticleData{});
			}

			mIsRunning = false;
			SetActive(false);
			ParticleRenderer::I->RemoveParticleSystem(mPSIdx);
			return;
		}
	}

	// �̹̼��� �� �ִ� ��쿡�� ��ƼŬ�� �߰��Ѵ�.
	const float createInterval = 1.f / mPSCD->Emission.RateOverTime;
	if (mPSCD->Emission.IsOn) {
		if (createInterval < mAccElapsed) {
			mAccElapsed -= createInterval;
			mPSGD.AddCount = mPSCD->MaxAddCount;
		}
	}

	// ��� ����Ʈ�� ���Ͽ� ��� �ð��� ������ count��ŭ �߰�
	for (int i = 0; i < mPSCD->Emission.Bursts.size(); ++i) {
		if (mBurstElapseds[i] - mPSCD->Emission.Bursts[i].Time >= (mPSCD->Duration * mPSCD->SimulationSpeed)) {
			if (mBurstRunnings[i])
				continue;

			// AddCount�� �ش� ����Ʈ�� Count �߰�
			mPSGD.AddCount += mPSCD->Emission.Bursts[i].Count;
			mBurstRunnings[i] = true;

			if (mPSCD->Looping) {
				mBurstElapseds[i] = 0.f;
				mBurstRunnings[i] = false;
			}
		}
	}

	mIsRunning = true;

	// �޸� ����
	FRAME_RESOURCE_MGR->CopyData(mPSIdx, mPSGD);
}

void ParticleSystem::OnDestroy()
{
	// ��ƼŬ �ý����� ���� �ִ� ��ü �Ҹ� �� ȣ��
	if (mIsDeprecated)
		return;

	// ���� �ִ� ��ü�� �Ҹ�Ǵ��� �ٷ� �Ҹ���� �ʵ��� ��ƼŬ �������� ������ ����
	Stop();
	mIsDeprecated = true;
	ParticleRenderer::I->AddParticleSystem(shared_from_this());
}

void ParticleSystem::Play()
{
	printf("%f, %f\n", mTarget->GetPosition().z, mTarget->GetPosition().x);
	// �÷��� �� ��ƼŬ �������� �߰� �� ��� �ð� �ʱ�ȭ
	SetActive(true);
	ParticleRenderer::I->AddParticleSystem(shared_from_this());
	mLoopingElapsed = 0.f;
	mStopElapsed = 0.f;
	mStartElapsed = 0.f;
	mAccElapsed = 0.f;

	mPSGD.TotalTime = 0.f;
	mPSGD.DeltaTime = 0.f;

	mIsStopCreation = false;
	mIsDeprecated = false;

	// ó�� �������ڸ��� ��ƼŬ�� �����ϱ� ����
	for (int i = 0; i < mPSCD->Emission.Bursts.size(); ++i) {
		mBurstElapseds[i] = mPSCD->Duration * mPSCD->SimulationSpeed;
		mBurstRunnings[i] = false;
	}
}

void ParticleSystem::Stop()
{
	// Stop �÷��� ����, ���� �÷��׶�� ���ٴ� ���� �˸� �÷���
	if (!mIsStopCreation) {
		mIsStopCreation = true;
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

void ParticleSystem::Save()
{
	std::string filePath = "Import/ParticleSystem/" + mPSCD->mName + ".xml";

	std::ofstream out(filePath);

	if (out) {
		boost::archive::xml_oarchive oa(out);
		oa << BOOST_SERIALIZATION_NVP(mPSCD);
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
	mParticleSystems.reserve(300);
	mDeprecations.reserve(300);
	mComputeShader = RESOURCE<Shader>("ComputeParticle");
	mAlphaShader = RESOURCE<Shader>("GraphicsParticle");
	mOneToOneShader = RESOURCE<Shader>("OneToOneBlend_GraphicsParticle");
	mAlphaStretchedShader = RESOURCE<Shader>("GraphicsStretchedParticle");
	mOneToOneStretchedShader = RESOURCE<Shader>("OneToOneBlend_GraphicsStretchedParticle");
}

void ParticleRenderer::AddParticleSystem(sptr<ParticleSystem> particleSystem)
{
	mParticleSystems.insert(std::make_pair(particleSystem->GetPSIdx(), particleSystem));

	// ��ƼŬ �ý����� ���� ���� ��ƼŬ�̶�� Deprecations �����̳ʿ��� �߰�
	if (particleSystem->IsDeprecated()) {
		mDeprecations.insert(std::make_pair(particleSystem->GetPSIdx(), particleSystem));
	}
}

void ParticleRenderer::RemoveParticleSystem(int particleSystemIdx)
{
	// ���� ���� ��ƼŬ �ý��� �ε���
	mRemoval.push(particleSystemIdx);
}

void ParticleRenderer::Update()
{
	// ��ƼŬ �ý����� ���� �ִ� ��ü�� ���� �Ǵ��� �ٷ� �Ҹ���� �ʵ��� ������Ʈ
	for (const auto& ps : mDeprecations) {
		ps.second->Update();
	}

	// ��ƼŬ ����
	while (!mRemoval.empty()) {
		int deprecated = mRemoval.front();
		mDeprecations.erase(deprecated);
		mParticleSystems.erase(deprecated);
		mRemoval.pop();
	}
}

void ParticleRenderer::Render() const
{
	// ��ǻƮ ���̴� ����
	mComputeShader->Set();
	for (const auto& ps : mParticleSystems) {
		if (!ps.second->IsRunning())
			continue;

		ps.second->ComputeParticleSystem();
	}

	// ���̴��� ���� ��� ���̴��� set ���� �ʴ´�.
	PSRenderMode prevMode = PSRenderMode::None;
	BlendType prevBlendType = BlendType::Alpha_Blend;
	for (const auto& ps : mParticleSystems) {
		if (!ps.second->IsRunning())
			continue;

		const PSRenderMode currMode = ps.second->GetPSCD()->Renderer.RenderMode;
		const BlendType currBlendType = ps.second->GetPSCD()->Renderer.BlendType;

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
