#pragma once

#pragma region ClassForwardDecl
class ModelObjectMesh;
#pragma endregion

#pragma region EnumClass
// must be matched with Light.hlsl LightType
enum class LightType {
	Spot        = 0,
	Directional = 1,
	Point       = 2
};
#pragma endregion


#pragma region Variable
// move to stdafx.h
#pragma endregion


#pragma region Struct
// move to stdafx.h
#pragma endregion


#pragma region Class
class Light {
private:
	sptr<ModelObjectMesh>	mVolumeMesh{};	// light volume mesh
	sptr<SceneLight>		mLights{};		// all lights in scene
	sptr<SceneLoadLight>	mLoadLights{};	// all load lights in scene

	size_t					mCurrLightCnt{};	// count of allocated light in scene

	std::unordered_map<std::string, const LightLoadInfo*> mLightModels{};	// �ϳ��� ���� �𵨿� ���� ���� ������ �������� ���� ������ �� ���
	
	std::set<std::string> mLightModelNames{};

public:
	Light();
	virtual ~Light();

	// [modelName]�� ���� LightInfo�� ��ȯ�Ѵ�.
	const LightLoadInfo* GetLightModel(const std::string& modelName) const;

	// [index]�� ���� LightInfo�� ��ȯ�Ѵ�.
	LightInfo* GetLight(int index) const { return &mLights->Lights[index]; }
	// ��ü ������ ��ȯ�Ѵ�.
	rsptr<SceneLight> GetSceneLights() const { return mLights; }

	UINT GetLightCount() const { return static_cast<UINT>(mLights->Lights.size()); }

public:
	// ���ο� ���� ���� �����Ѵ�.
	void InsertLightModel(const std::string& name, const LightLoadInfo* light) { mLightModels.insert(std::make_pair(name, light)); }

	void BuildLights(FILE* file);
	void BuildLights();

	// ������ ���� �޽��� �������Ѵ�.
	void Render();

private:

	// ������ Sunlight�� �����Ѵ�.
	void SetSunlight();

	// ���� �� �������� ���� �𵨵��� �ҷ��´�.
	void LoadLightModels();
	// [file]���� ���� ���� �ҷ����� �Ҵ��Ѵ�.
	void LoadLightObjects(FILE* file);
};
#pragma endregion