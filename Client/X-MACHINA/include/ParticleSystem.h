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
	int mParticleSystemIndex = -1;	// ��ƼŬ �ý��� ������ ���� �ε���
	float mCreateInterval = 0.005f;	// ��ƼŬ ���� �ֱ�

	ParticleSystemData	mParticleSystemData{};		// ��� ��ƼŬ�� ���������� ����Ǵ� ������
	uptr<UploadBuffer<ParticleData>> mParticles;	// ��� ��ƼŬ�� Ư�������� ����Ǵ� ������

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