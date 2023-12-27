#include "stdafx.h"
#include "Light.h"
#include "DXGIMgr.h"

#include "Scene.h"
#include "FileMgr.h"

std::set<std::string> lightModelNames = { "apache_high_light", "tank_head_light", "tank_high_light" };



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////

Light::Light()
{
	mLights = std::make_shared<LIGHTS>();
	XMStoreFloat4(&mLights->fogColor, Colors::Gray);
}


Light::~Light()
{
	for (auto& light : mLightModels) {
		if (lightModelNames.contains(light.first)) {
			delete light.second;
		}
	}

	lightModelNames.clear();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////

const LIGHT* Light::GetLightModel(const std::string& modelName)
{
	assert(mLightModels.contains(modelName));

	return mLightModels[modelName];
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////

void Light::SetSunlight()
{
	LIGHT& light = mLights->mLights[0];
	light.mType = static_cast<int>(LightType::Directional);
	light.mAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.mDiffuse = XMFLOAT4(1.0f, 0.956f, 0.839f, 1.0f);
	light.mSpecular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	light.mDirection = XMFLOAT3(-0.6f, -0.6f, 0.3f);
}


void Light::SetMoonLight()
{
	LIGHT& light = mLights->mLights[0];
	light.mType = static_cast<int>(LightType::Directional);
	light.mAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.mDiffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.mSpecular = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	light.mDirection = XMFLOAT3(0.6f, -0.6f, -0.3f);
}


void Light::SetGlobalLight(GlobalLight globalLight)
{
	switch (globalLight) {
	case GlobalLight::Sunlight:
		SetSunlight();
		break;
	case GlobalLight::MoonLight:
		SetMoonLight();
		break;
	default:
		assert(0);
		break;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

//////////////////* DirectX *//////////////////
void Light::CreateShaderVariables()
{
	UINT cubeLightBytes = ((sizeof(LIGHTS) + 255) & ~255); //256ÀÇ ¹è¼ö
	::CreateBufferResource(nullptr, cubeLightBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, mCBLights);
	mCBLights->Map(0, nullptr, (void**)&mCBMappedLights);
}


void Light::UpdateShaderVariables()
{
	::memcpy(mCBMappedLights, mLights.get(), sizeof(LIGHTS));

	D3D12_GPU_VIRTUAL_ADDRESS cbLightsGpuVirtualAddress = mCBLights->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(crntScene->GetRootParamIndex(RootParam::Light), cbLightsGpuVirtualAddress);
}


void Light::ReleaseShaderVariables()
{
	if (mCBLights) {
		mCBLights->Unmap(0, nullptr);
	}
}


//////////////////* Build *//////////////////

void Light::LoadLightModels()
{
	for (auto& name : lightModelNames) {
		LIGHT* light = new LIGHT;
		FileMgr::LoadLightFromFile("Models/Lights/" + name + ".bin", &light);
		mLightModels.insert(std::make_pair(name, light));
	}
}


void Light::LoadLightObjects(FILE* file)
{
	std::string token{};
	std::string name{};

	UINT nReads = 0;

	int lightCount;
	::ReadUnityBinaryString(file, token); // "<Lights>:"
	nReads = (UINT)::fread(&lightCount, sizeof(int), 1, file);

	++lightCount;
	assert(lightCount <= MAX_SCENE_LIGHTS);

	int sameLightCount{};
	LIGHT* modelLight{};
	for (size_t i = 1; i < lightCount; ++i) {
		LIGHT* light = &mLights->mLights[i];
		light->mIsEnable = true;

		if (sameLightCount <= 0) {
			::ReadUnityBinaryString(file, token); //"<FileName>:"

			std::string fileName{};
			::ReadUnityBinaryString(file, fileName);

			FileMgr::LoadLightFromFile("Models/Lights/" + fileName + ".bin", &light);
			InsertLight(fileName, light);

			modelLight = light;

			::ReadUnityBinaryString(file, token); //"<Transforms>:"
			::fread(&sameLightCount, sizeof(int), 1, file);
		}

		*light = *modelLight;

		Vec4x4 transform;
		::fread(&transform, sizeof(float), 16, file); //Transform

		light->mPosition.x = transform._41;
		light->mPosition.y = transform._42;
		light->mPosition.z = transform._43;

		light->mDirection.x = transform._31;
		light->mDirection.y = transform._32;
		light->mDirection.z = transform._33;

		--sameLightCount;
	}
}


void Light::BuildLights(FILE* file)
{
	LoadLightObjects(file);
	LoadLightModels();

	// 0 is global light
	mLights->mLights[0].mIsEnable = true;
	SetSunlight();
}



//////////////////* Others *//////////////////

LIGHT_RANGE Light::AlloLight(size_t count)
{
	mCrntLightCount += count;
	assert(mCrntLightCount <= MAX_SCENE_LIGHTS);

	LIGHT_RANGE range;
	range.lights = &mLights;
	range.begin = mCrntLightCount - count;
	range.end = mCrntLightCount;

	return range;
}