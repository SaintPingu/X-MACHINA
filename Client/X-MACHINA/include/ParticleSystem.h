#pragma once

#pragma region Include
#include "Component.h"
#include "UploadBuffer.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Material;
class Texture;
#pragma endregion


#pragma region Class
/* particle system component */
class ParticleSystem : public Component {
	COMPONENT(ParticleSystem, Component)

	friend class ParticleSystemObject;

private:
	int mParticleSystemIndex = -1;	// 파티클 시스템 구조적 버퍼 인덱스
	float mCreateInterval = 0.005f;	// 파티클 생성 주기

	ParticleSystemData	mParticleSystemData{};		// 모든 파티클에 공통적으로 적용되는 데이터
	uptr<UploadBuffer<ParticleData>> mParticles;	// 모든 파티클에 특수적으로 적용되는 데이터

public:
	virtual void Awake() override;
	virtual void Update() override;
	virtual void OnDestroy() override;
	void Render();

public:
	void SetTexture(int textureIndex) { mParticleSystemData.TextureIndex = textureIndex; }

private:
	void UpdateComputeShaderVars();	
	void UpdateGraphicsShaderVars();
};
#pragma endregion