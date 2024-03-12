#include "stdafx.h"
#include "ParticleSystem.h"
#include "Timer.h"
#include "ResourceMgr.h"
#include "Mesh.h"





#pragma region ParticleSystem
void ParticleSystem::Awake()
{
	base::Awake();

	mParticleSystemData.AddCount	= 0;
	mParticleSystemData.MaxCount	= 1000;
	mParticleSystemData.DeltaTime	= 0.f;
	mParticleSystemData.AccTime		= 0.f;
	mParticleSystemData.MinLifeTime = 0.5f;
	mParticleSystemData.MaxLifeTime = 5.f;
	mParticleSystemData.MinSpeed	= 100;
	mParticleSystemData.MaxSpeed	= 50;
	mParticleSystemData.StartScale	= 10.f;
	mParticleSystemData.EndScale	= 0.f;
}

void ParticleSystem::Update()
{
	mParticleSystemData.AccTime += DeltaTime();
	mParticleSystemData.DeltaTime = DeltaTime();

	mParticleSystemData.AddCount = 0;
	if (mCreateInterval < mParticleSystemData.AccTime) {
		mParticleSystemData.AccTime = mParticleSystemData.AccTime - mCreateInterval;
		mParticleSystemData.AddCount = 1;
	}
}
#pragma endregion





#pragma region ParticleSystemObject
ParticleSystemObject::ParticleSystemObject() : Object()
{
	mParticleSystem = AddComponent<ParticleSystem>();
}

void ParticleSystemObject::Update()
{
	base::Update();					// update base
	Transform::UpdateShaderVars();	// update objectCB
	mParticleSystem->Update();		// update particle system 
	UpdateShaderVars();				// update particle system object
}

void ParticleSystemObject::OnDestroy()
{
	Transform::OnDestroy();


}

void ParticleSystemObject::UpdateShaderVars()
{

}

void ParticleSystemObject::Render()
{
	res->Get<ModelObjectMesh>("Point")->RenderInstanced(mParticleSystem->mParticleSystemData.MaxCount);
}



#pragma endregion

