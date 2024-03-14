#pragma once

#pragma region Define
#define pr ParticleRenderer::Inst()
#pragma endregion

#pragma region Include
#include "Component.h"
#include "UploadBuffer.h"
#pragma endregion

struct ParticleSystemCPUData {
public:
	/* my value */
	bool				IsStop = true;
	int					MaxAddCount = 1;			// 1틱당 파티클 생성 개수
	float				CreateInterval = 0.005f;	// 파티클 생성 주기

	/* unity value */
	float				Duration = 1.f;
	bool				IsLooping = true;
	bool				Prewarm = false;
	float				StartDelay = 0.f;

	/* elapsed time */
	float				StopElapsed = 0.f;
	float				StartElapsed = 0.f;
	float				LoopingElapsed = 0.f;
};
#pragma endregion

#pragma region Class
/* particle system component */
class ParticleSystem : public Component, public std::enable_shared_from_this<ParticleSystem> {
	COMPONENT(ParticleSystem, Component)

private:
	Transform*				mTarget = mObject;		// 파티클을 부착시킬 타겟
	int						mPSIdx = -1;			// 파티클 시스템 구조적 버퍼 인덱스
	ParticleSystemCPUData	mPSCD{};				// 모든 파티클에 공통적으로 적용되는 CPU 데이터
	ParticleSystemGPUData	mPSGD{};				// 모든 파티클에 공통적으로 적용되는 GPU 데이터
	uptr<UploadBuffer<ParticleData>> mParticles;	// 개별 파티클에 특수적으로 적용되는 GPU 데이터

public:
#pragma region Getter
	ParticleSystemCPUData& GetPSCD() { return mPSCD; }
	ParticleSystemGPUData& GetPSGD() { return mPSGD; }
	int GetPSIdx() const { return mPSIdx; }
#pragma endregion

#pragma region Setter
	void SetTarget(const std::string& frameName);
#pragma endregion

public:
	virtual void Awake() override;
	virtual void Update() override;
	virtual void OnDestroy() override;

public:
	void Play();
	void Stop();
	void PlayToggle();

	void ComputeParticleSystem() const;
	void RenderParticleSystem() const;
};


class ParticleRenderer : public Singleton<ParticleRenderer> {
	friend Singleton;

private:
	std::unordered_map<int, sptr<ParticleSystem>> mParticleSystems;

public:
#pragma region C/Dtor
	ParticleRenderer() = default;
	virtual ~ParticleRenderer() = default;
#pragma endregion

public:
	void Init();
	void AddParticleSystem(sptr<ParticleSystem> particleSystem);
	void RemoveParticleSystem(int particleSystemIdx);
	void Render() const;
};
#pragma endregion
