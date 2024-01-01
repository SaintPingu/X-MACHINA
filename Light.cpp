#include "stdafx.h"
#include "Light.h"
#include "DXGIMgr.h"

#include "Scene.h"
#include "FileIO.h"

namespace {
	constexpr int gkSunLightIdx = 0;

	std::set<std::string> lightModelNames = { "apache_high_light", "tank_head_light", "tank_high_light" };
}




Light::Light()
{
	mLights = std::make_shared<SceneLight>();
	XMStoreFloat4(&mLights->FogColor, Colors::Gray);
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

const LightInfo* Light::GetLightModel(const std::string& modelName) const
{
	assert(mLightModels.contains(modelName));

	return mLightModels.at(modelName);
}



void Light::BuildLights(FILE* file)
{
	LoadLightObjects(file);
	LoadLightModels();

	mLights->Lights[gkSunLightIdx].IsEnable = true;
	SetSunlight();
}


void Light::CreateShaderVars()
{
	UINT cubeLightBytes = ((sizeof(SceneLight) + 255) & ~255); //256ÀÇ ¹è¼ö
	D3DUtil::CreateBufferResource(nullptr, cubeLightBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, mCB_Lights);
	mCB_Lights->Map(0, nullptr, (void**)&mCBMap_Lights);
}

void Light::UpdateShaderVars()
{
	::memcpy(mCBMap_Lights, mLights.get(), sizeof(SceneLight));

	D3D12_GPU_VIRTUAL_ADDRESS cbLightsGpuVirtualAddress = mCB_Lights->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(scene->GetRootParamIndex(RootParam::Light), cbLightsGpuVirtualAddress);
}

void Light::ReleaseShaderVars()
{
	if (mCB_Lights) {
		mCB_Lights->Unmap(0, nullptr);
	}
}



void Light::SetSunlight()
{
	LightInfo& light     = mLights->Lights[gkSunLightIdx];
	light.Type           = static_cast<int>(LightType::Directional);
	light.Ambient        = Vec4(0.1f, 0.1f, 0.1f, 1.f);
	light.Diffuse        = Vec4(1.f, 0.956f, 0.839f, 1.f);
	light.Specular       = Vec4(0.5f, 0.5f, 0.5f, 1.f);
	light.Direction      = Vec3(-0.6f, -0.6f, 0.3f);
}

void Light::LoadLightModels()
{
	for (auto& name : lightModelNames) {
		LightInfo* light = new LightInfo;
		FileIO::LoadLightFromFile("Models/Lights/" + name + ".bin", &light);
		mLightModels.insert(std::make_pair(name, light));
	}
}

void Light::LoadLightObjects(FILE* file)
{
	std::string token{};
	std::string name{};

	int lightCount;
	FileIO::ReadString(file, token); // "<Lights>:"
	FileIO::ReadVal(file, lightCount);

	++lightCount;
	assert(lightCount <= gkMaxSceneLight);

	int sameLightCount{};
	LightInfo* modelLight{};
	for (size_t i = 1; i < lightCount; ++i) {
		LightInfo* light = &mLights->Lights[i];
		light->IsEnable  = true;

		if (sameLightCount <= 0) {
			FileIO::ReadString(file, token); //"<FileName>:"

			std::string fileName{};
			FileIO::ReadString(file, fileName);

			FileIO::LoadLightFromFile("Models/Lights/" + fileName + ".bin", &light);
			InsertLightModel(fileName, light);

			modelLight = light;

			FileIO::ReadString(file, token); //"<Transforms>:"
			FileIO::ReadVal(file, sameLightCount);
		}

		*light = *modelLight;

		Vec4x4 transform;
		FileIO::ReadVal(file, transform);

		light->Position.x = transform._41;
		light->Position.y = transform._42;
		light->Position.z = transform._43;

		light->Direction.x = transform._31;
		light->Direction.y = transform._32;
		light->Direction.z = transform._33;

		--sameLightCount;
	}
}
