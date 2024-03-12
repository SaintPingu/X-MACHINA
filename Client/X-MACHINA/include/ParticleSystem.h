#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
struct ParticleSystemData;
#pragma endregion


#pragma region Class
/* particle system component */
class ParticleSystem : public Component {
	COMPONENT(ParticleSystem, Component)

	friend class ParticleSystemObject;

private:
	float mCreateInterval = 0.005f;
	ParticleSystemData mParticleSystemData;

public:
	virtual void Awake() override;

public:
	void Update();
};


/* object with particle system */
class ParticleSystemObject : public Object {
	using base = Object;

private:
	sptr<ParticleSystem> mParticleSystem{};

public:
	ParticleSystemObject();
	virtual ~ParticleSystemObject() = default;

public:
	virtual void Update() override;
	virtual void OnDestroy() override;

public:
	void UpdateShaderVars();
	void Render();
};
#pragma endregion
