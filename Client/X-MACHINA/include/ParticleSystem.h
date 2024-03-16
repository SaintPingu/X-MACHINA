#pragma once

#pragma region Include
#include "Component.h"
#include "UploadBuffer.h"
#pragma endregion


#pragma region Define
#define pr ParticleRenderer::Inst()
#pragma endregion


#pragma region Struct
struct ParticleSystemCPUData {
public:
	/* my value */
	bool	IsStop = true;
	int		MaxAddCount = 1;			// 1틱당 파티클 생성 개수
	float	CreateInterval = 0.005f;	// 파티클 생성 주기

	/* unity value */
	float	Duration = 1.f;
	bool	IsLooping = true;
	bool	Prewarm = false;
	float	StartDelay = 0.f;
	Vec2	StartLifeTime = Vec2{ 1.f };
	Vec2	StartSpeed = Vec2{ 1.f };
	Vec2	StartSize = Vec2{ 0.1f };

	bool	PlayOnAwake = true;

	/* elapsed time */
	float	StopElapsed = 0.f;
	float	StartElapsed = 0.f;
	float	LoopingElapsed = 0.f;
};

struct ParticleSystemGPUData {
	Vec3	WorldPos = { 0.f, 0.f, 0.f };
	int		AddCount = 0;

	int		MaxCount = 1000;
	float	DeltaTime = 0.f;
	float	AccTime = 0.f;
	float	MinLifeTime = 1.f;

	float	MaxLifeTime = 1.f;
	float	MinSpeed = 100;
	float	MaxSpeed = 50;
	int		TextureIndex = -1;

	Vec4	Color = { 1.f, 1.f, 1.f, 1.f };

	Vec2	StartLifeTime = Vec2{ 0.f };
	Vec2	StartSpeed = Vec2{ 0.f };
	Vec2	StartSize = Vec2{ 0.f };
	Vec2	Padding;
};

struct ParticleData {
	Vec3	WorldPos = { 0.f, 0.f, 0.f };
	float	CurTime = 0.f;
	Vec3	LocalPos = { 0.f, 0.f, 0.f };
	float	LifeTime = 0.f;
	Vec3	WorldDir = { 0.f, 0.f, 0.f };
	int	    Alive = 0;
	Vec4	Color = { 1.f, 1.f, 1.f, 1.f };
	Vec2	StartEndScale = { 0.f, 0.f };
	int		TextureIndex = -1;
	float	StartSpeed = 0.f;
};

struct ParticleSharedData {
	int	    AddCount = 0;
	Vec3	Padding;
};
#pragma endregion

#pragma region Class
/* particle system component */
class ParticleSystem : public Component, public std::enable_shared_from_this<ParticleSystem> {
	COMPONENT(ParticleSystem, Component)

private:
	Transform*				mTarget = mObject;		// 파티클을 부착시킬 타겟
	bool					mIsDeprecated = false;	// 파티클 시스템 삭제 예정 플래그
	int						mPSIdx = -1;			// 파티클 시스템 구조적 버퍼 인덱스
	ParticleSystemCPUData	mPSCD{};				// 모든 파티클에 공통적으로 적용되는 CPU 데이터
	ParticleSystemGPUData	mPSGD{};				// 모든 파티클에 공통적으로 적용되는 GPU 데이터
	uptr<UploadBuffer<ParticleData>> mParticles;	// 개별 파티클에 특수적으로 적용되는 GPU 데이터

public:
#pragma region Getter
	ParticleSystemCPUData& GetPSCD() { return mPSCD; }
	ParticleSystemGPUData& GetPSGD() { return mPSGD; }
	int GetPSIdx() const { return mPSIdx; }
	bool IsDeprecated() const { return mIsDeprecated; }
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

	void ReturnIndex();
};


class ParticleRenderer : public Singleton<ParticleRenderer> {
	friend Singleton;

private:
	std::unordered_map<int, sptr<ParticleSystem>> mParticleSystems;
	std::unordered_map<int, sptr<ParticleSystem>> mDeprecations;
	std::queue<int> mRemoval;

public:
#pragma region C/Dtor
	ParticleRenderer() = default;
	virtual ~ParticleRenderer() = default;
#pragma endregion

public:
	void Init();
	void AddParticleSystem(sptr<ParticleSystem> particleSystem);
	void RemoveParticleSystem(int particleSystemIdx);

	void Update();
	void Render() const;
};
#pragma endregion
