#pragma once

#pragma region EnumClass
// must be matched with Light.hlsl LightType
enum class LightType {
	Spot        = 0,
	Directional = 1,
	Point       = 2
};
#pragma endregion


#pragma region Variable
constexpr int gkMaxSceneLight = 32;	// ���� ������ �� �ִ� ������ �ִ� ����. Light.hlsl�� �����ؾ� �Ѵ�.
#pragma endregion


#pragma region Struct
// must be matched with Light.hlsl LightInfo
struct LightInfo {
	Vec4	Ambient{};
	Vec4	Diffuse{};
	Vec4	Specular{};

	Vec3	Position{};
	float	Falloff{};

	Vec3	Direction{};
	float	Theta{};

	Vec3	Attenuation{};
	float	Phi{};

	float	Range{};
	float	Padding{};
	int		Type{};
	bool	IsEnable{};
};

// must be matched with Light.hlsl cbLights
struct SceneLight {
	std::array<LightInfo, gkMaxSceneLight> Lights{};

	Vec4	GlobalAmbient{};

	Vec4	FogColor{};
	float	FogStart = 100.f;
	float	FogRange = 300.f;
};
#pragma endregion


#pragma region Class
class Light {
private:
	sptr<SceneLight> mLights{};	// all lights in scene

	ComPtr<ID3D12Resource>	mCB_Lights{};
	SceneLight*				mCBMap_Lights{};
	size_t					mCurrLightCnt{};	// count of allocated light in scene

	std::unordered_map<std::string, const LightInfo*> mLightModels{};	// �ϳ��� ���� �𵨿� ���� ���� ������ �������� ���� ������ �� ���

public:
	Light();
	virtual ~Light();

	// [modelName]�� ���� LightInfo�� ��ȯ�Ѵ�.
	const LightInfo* GetLightModel(const std::string& modelName) const;
	// [index]�� ���� LightInfo�� ��ȯ�Ѵ�.
	LightInfo* GetLight(int index) const { return &mLights->Lights[index]; }

public:

	// ���ο� ���� ���� �����Ѵ�.
	void InsertLightModel(const std::string& name, const LightInfo* light) { mLightModels.insert(std::make_pair(name, light)); }

	void BuildLights(FILE* file);

	void CreateShaderVars();
	void UpdateShaderVars();
	void ReleaseShaderVars();

private:

	// ������ Sunlight�� �����Ѵ�.
	void SetSunlight();

	// ���� �� �������� ���� �𵨵��� �ҷ��´�.
	void LoadLightModels();
	// [file]���� ���� ���� �ҷ����� �Ҵ��Ѵ�.
	void LoadLightObjects(FILE* file);
};
#pragma endregion