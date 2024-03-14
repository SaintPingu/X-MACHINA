#pragma once

#pragma region Include
#include "Component.h"
#include "UploadBuffer.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Material;
class Texture;
struct ParticleSystemData;
#pragma endregion


#pragma region Class
/* particle system component */
class ParticleSystem : public Component {
	COMPONENT(ParticleSystem, Component)

	friend class ParticleSystemObject;

private:
	float mCreateInterval = 0.005f;
	ParticleSystemData	mParticleSystemData;
	uptr<UploadBuffer<ParticleData>> mParticles;

public:
	virtual void Awake() override;

public:
	void Update();
};


/* object with particle system */
class ParticleSystemObject : public Object {
	using base = Object;

private:
	int mParticleSystemIndex = -1;
	sptr<ParticleSystem> mParticleSystem{};

public:
	ParticleSystemObject(Vec3 worldPos);
	virtual ~ParticleSystemObject() = default;

public:
	virtual void Awake() override;
	virtual void Update() override;
	virtual void OnDestroy() override;

public:
	void SetTexture(rsptr<Texture> texture);

public:
	// 컴퓨트 쉐이더에서 사용하는 상수 
	void UpdateComputeShaderVars();
	// 그래픽스 쉐이더에서 사용하는 상수
	void UpdateGraphicsShaderVars();

	void Render();
};
#pragma endregion
