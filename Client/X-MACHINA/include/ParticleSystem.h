#pragma once

#pragma region Include
#include "Component.h"
#include "UploadBuffer.h"
#pragma endregion

#pragma region Class
/* particle system component */
class ParticleSystem : public Component {
	COMPONENT(ParticleSystem, Component)

private:
	Transform*	mTarget = mObject;			// 파티클을 부착시킬 타겟
	int			mPSIdx = -1;				// 파티클 시스템 구조적 버퍼 인덱스
	float		mCreateInterval = 0.005f;	// 파티클 생성 주기

	ParticleSystemData mPSData{};					// 모든 파티클에 공통적으로 적용되는 데이터
	uptr<UploadBuffer<ParticleData>> mParticles;	// 모든 파티클에 특수적으로 적용되는 데이터

public:
#pragma region Getter
	ParticleSystemData& GetPSData() { return mPSData; }
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
	void Render() const;

private:
	void UpdateComputeShaderVars() const;
	void UpdateGraphicsShaderVars() const;
};
#pragma endregion
