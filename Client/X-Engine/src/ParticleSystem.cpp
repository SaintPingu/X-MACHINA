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


void ParticleSystem::Awake()
{
	base::Awake();

#pragma region Init_ParticleSystem
	// ��ƼŬ �ý��� �ε����� ��� ���� �뵵
	frmResMgr->CopyData(mPSIdx, mPSGD);
#pragma endregion

#pragma region Init_Particles
	// ��ƼŬ �ִ� ������ŭ ����
	mParticles = std::make_unique<UploadBuffer<ParticleData>>(device.Get(), mPSCD.MaxParticles, false);
#pragma endregion

	// ���� ������ڸ��� �÷���
	if (mPSCD.PlayOnAwake) {
		Play();
	}
}


void ParticleSystem::Update()
{
	// ���� ���α׷����� ������ ��(PSCD)�� GPU�� �Ѱ��ֱ� ���Ͽ� PSGD ������Ʈ 
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

	// ���� ��尡 Stretched�� ��� StartSize3D�� ����Ͽ� y���� ���� ����
	if (mPSCD.Renderer.RenderMode == PSRenderMode::StretchedBillboard) {
		// StartSize3D�� ������� �ʴ� ��� StartSize ���� StartSize3D�� ����
		if (mPSCD.StartSize3D.w == 0.f) {
			mPSGD.StartSize3D.x = mPSCD.StartSize.x;
			mPSGD.StartSize3D.y = mPSCD.StartSize.x * mPSCD.Renderer.LengthScale;
		}
		// StartSize3D�� ����� ��� StartSize3D ���� �״�� ���
		else {
			mPSGD.StartSize3D.x = mPSCD.StartSize3D.x;
			mPSGD.StartSize3D.y = mPSGD.StartSize3D.y * mPSCD.Renderer.LengthScale;
		}
		// StartSize3D�� ����� ���̱� ������ w ���� 1�� ����
		mPSGD.StartSize3D.w = 1.f;
	}

	// �ùķ��̼� �ӵ��� ���� ��Ÿ Ÿ�ӿ� �ùķ��̼� �ӵ� ����
	const float kSimulationDeltaTime = DeltaTime() * mPSCD.SimulationSpeed;

#pragma region Check_Looping
	// looping�� ������� ���� ��� Duration �ð� ���ȸ� ��ƼŬ ������Ʈ
	if (!mPSCD.Looping && mPSCD.LoopingElapsed >= mPSCD.Duration) {
		Stop();
	}
#pragma endregion

#pragma region Check_StartDelay
	// StartDelay ���ĺ��� ����
	mPSCD.StartElapsed += kSimulationDeltaTime;
	if (mPSCD.StartElapsed <= mPSCD.StartDelay) {
		return;
	}
#pragma endregion

#pragma region Check_StopDelay
	// ��ƼŬ �ý��� ������ ����� ���
	if (mPSCD.IsStop) {
		mPSCD.StopElapsed += kSimulationDeltaTime;
		// Prewarm ���� ���� true�̸� �ٷ� ����, false�̸� ���� �ֱ� �ִ� �ð��� ��ٸ� �� ����
		if (mPSCD.StopElapsed >= (!mPSCD.Prewarm ? mPSGD.StartLifeTime.y : 0)) {
			SetActive(false);

			pr->RemoveParticleSystem(mPSIdx);

			// ��ü �Ҹ� ���� ��ƼŬ�� �Ҹ��ؾ� �ϴ� ��� �������� �ε��� ��ȯ
			//if (mIsDeprecated)
			//	ReturnIndex();

			return;
		}
	}
#pragma endregion

#pragma region Update
	// Ÿ�� ��ġ ���� �� �ùķ��̼� ��Ÿ Ÿ�� ����
	mPSGD.WorldPos = mTarget->GetPosition();
	mPSGD.TotalTime += kSimulationDeltaTime;
	mPSGD.DeltaTime = kSimulationDeltaTime;
	mPSCD.LoopingElapsed += kSimulationDeltaTime;
	mPSCD.AccTime += kSimulationDeltaTime;
	mPSCD.Emission.UpdateDeltaTime(kSimulationDeltaTime);

	// ��ƼŬ ���� �ð��� �Ǹ� AddCount ����
	mPSGD.AddCount = 0;
	const float createInterval = mPSCD.Emission.IsOn ? (1.f / mPSCD.Emission.RateOverTime) : 0.f;
	if (createInterval < mPSCD.AccTime) {
		// ���� �ð� �ʱ�ȭ �� AddCount ����
		mPSCD.AccTime = mPSCD.AccTime - (1.f / mPSCD.Emission.RateOverTime);
		mPSGD.AddCount = mPSCD.IsStop ? 0 : mPSCD.MaxAddCount;
	}

	// ��� ����Ʈ�� ���Ͽ� ��� �ð��� ������ count��ŭ �߰�
	for (auto& burst : mPSCD.Emission.Bursts) {
		if (burst.BurstElapsed >= mPSCD.Duration) {
			// AddCount�� �ش� ����Ʈ�� Count �߰�
			mPSGD.AddCount += burst.Count;
			burst.BurstElapsed = 0.f;
		}
	}

	// �޸� ����
	frmResMgr->CopyData(mPSIdx, mPSGD);
#pragma endregion
}

void ParticleSystem::OnDestroy()
{
	// ��ƼŬ �ý����� ���� �ִ� ��ü �Ҹ� �� ȣ��
	if (mIsDeprecated)
		return;

	// ���� �ִ� ��ü�� �Ҹ�Ǵ��� �ٷ� �Ҹ���� �ʵ��� ��ƼŬ �������� ������ ����
	//Stop();
	mIsDeprecated = true;
	pr->AddParticleSystem(shared_from_this());
}

void ParticleSystem::Play()
{
	// �÷��� �� ��ƼŬ �������� �߰� �� ��� �ð� �ʱ�ȭ
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
	// Stop �÷��� ����, ���� �÷��׶�� ���ٴ� ���� �˸� �÷���
	if (!mPSCD.IsStop) {
		mPSCD.IsStop = true;
	}
}

void ParticleSystem::PlayToggle()
{
	// �÷��� ���
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
		ps.second->ComputeParticleSystem();
	}

	// ���̴��� ���� ��� ���̴��� set ���� �ʴ´�.
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
