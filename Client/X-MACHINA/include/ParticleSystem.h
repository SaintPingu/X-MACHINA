#pragma once

#pragma region Include
#include "Component.h"
#include "UploadBuffer.h"
#pragma endregion


#pragma region Define
#define pr ParticleRenderer::Inst()
#pragma endregion

#pragma region EnumClass
enum class PSColorOption : UINT32 {
	Color = 0,
	Gradient,
	RandomBetweenTwoColors,
	RandomBetweenTwoGradient,
};

enum class SimulationSpace : UINT32 {
	Local = 0,
	World,
};
#pragma endregion


#pragma region Struct
struct PSColor {
	Vec4 FirstColor = Vec4{ 1.f };
	Vec4 SecondColor = Vec4{ 1.f };
	Vec4 FirstGradient = Vec4{ 1.f };
	Vec4 SecondGradient = Vec4{ 1.f };
	PSColorOption ColorOption = PSColorOption::Color;
	Vec3 Padding;

	void SetColor(PSColorOption colorOption, Vec4 value1, Vec4 value2 = Vec4{ 1.f }, Vec4 value3 = Vec4{ 1.f }, Vec4 value4 = Vec4{ 1.f }) {
		ColorOption = colorOption;
		switch (colorOption)
		{
		case PSColorOption::Color:
			FirstColor = value1;
			break;
		case PSColorOption::Gradient:
			FirstColor = value1;
			FirstGradient = value2;
			break;
		case PSColorOption::RandomBetweenTwoColors:
			FirstColor = value1;
			SecondColor = value2;
			break;
		case PSColorOption::RandomBetweenTwoGradient:
			FirstColor = value1;
			SecondColor = value2;
			FirstGradient = value3;
			SecondGradient = value4;
			break;
		default:
			break;
		}
	}
};

struct ParticleSystemCPUData {
public:
	/* my value */
	bool			 IsStop = true;
	int				 MaxAddCount = 1;		// 한번에 생성되는 파티클 개수
	int				 RateOverTime = 200;	// 초당 생성되는 파티클 개수
	int				 TextureIndex = -1;
	float			 AccTime = 0.f;

	/* elapsed time */
	float			 StopElapsed = 0.f;
	float			 StartElapsed = 0.f;
	float			 LoopingElapsed = 0.f;

	/* unity value */
	float			 Duration = 1.f;
	bool			 Looping = true;
	bool			 Prewarm = true;
	float			 StartDelay = 0.f;
	Vec2			 StartLifeTime = Vec2{ 1.f };
	Vec2			 StartSpeed = Vec2{ 1.f };
	Vec4			 StartSize3D = Vec4{ 0.1f, 0.1f, 0.1f, 0.f};	// w값이 0이면 StartSize3D를 사용하지 않음
	Vec2			 StartSize = Vec2{ 0.1f };
	Vec4			 StartRotation3D = Vec4{ 0.f, 0.f, 0.f, 0.f };	// w값이 0이면 StartRotation3D를 사용하지 않음
	Vec2			 StartRotation = Vec2{ 0.f };
	PSColor			 StartColor{};
	float			 GravityModifier = 0.f;
	SimulationSpace  SimulationSpace = SimulationSpace::World;
	float			 SimulationSpeed = 1.f;
	bool			 PlayOnAwake = true;
	int				 MaxParticles = 1000;
};

struct ParticleSystemGPUData {
	Vec3			 WorldPos{};
	int				 TextureIndex{};
	Vec4			 Color{};
					 
	int				 AddCount{};
	int				 MaxParticles{};
	float			 DeltaTime{};
	float			 TotalTime{};
					 
	Vec2			 StartLifeTime{};
	Vec2			 StartSpeed{};
	Vec4			 StartSize3D{};
	Vec4			 StartRotation3D{};
	Vec2			 StartSize{};
	Vec2			 StartRotation{};
	PSColor			 StartColor{};
	float			 GravityModifier{};
	SimulationSpace  SimulationSpace{};
	float			 SimulationSpeed{};
	float			 Padding{};
};

struct ParticleData {
	Vec3			 StartPos{};
	float			 CurTime{};
	Vec3			 LocalPos{};
	float			 LifeTime{};
	Vec3			 WorldPos{};
	int				 Alive{};
	Vec3			 WorldDir{};
	int				 TextureIndex{};
	Vec4			 StartColor{};
	Vec2			 StartSize{};
	float			 StartSpeed{};
	int				 Padding{};
	Vec3			 StartRotation{};
	int				 Padding2{};
};

struct ParticleSharedData {
	int				 AddCount{};
	Vec3			 Padding{};
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
