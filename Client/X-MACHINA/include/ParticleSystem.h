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
	Transform*	mTarget = mObject;			// ��ƼŬ�� ������ų Ÿ��
	int			mPSIdx = -1;				// ��ƼŬ �ý��� ������ ���� �ε���
	float		mCreateInterval = 0.005f;	// ��ƼŬ ���� �ֱ�

	ParticleSystemData mPSData{};					// ��� ��ƼŬ�� ���������� ����Ǵ� ������
	uptr<UploadBuffer<ParticleData>> mParticles;	// ��� ��ƼŬ�� Ư�������� ����Ǵ� ������

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
