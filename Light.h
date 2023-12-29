#pragma once

#define MAX_SCENE_LIGHTS	32

enum class GlobalLight {
	Sunlight = 0,
};

enum class LightType {
	Spot		= 0,
	Directional	= 1,
	Point		= 2,
	Area		= 3,
	Rectangle	= 3,
	Disc		= 4
};





struct LIGHT
{
	Vec4 mAmbient{};
	Vec4 mDiffuse{};
	Vec4 mSpecular{};
	Vec3 mPosition{};

	float mFalloff{};
	Vec3 mDirection{};
	float mTheta{};
	Vec3 mAttenuation{};
	float mPhi{};
	bool mIsEnable{};
	int mType{};
	float mRange{};
	float mPadding{};
};

struct LIGHTS
{
	LIGHT mLights[MAX_SCENE_LIGHTS]{};
	Vec4 mGlobalAmbient{};

	Vec4 fogColor{};
	float fogStart{ 100.f };
	float fogRange{ 300.f };
};

struct LIGHT_RANGE {
	sptr<LIGHTS>* lights{};
	size_t begin{};
	size_t end{};

	LIGHT& Get(size_t i) { return (*lights)->mLights[i]; }
};






class Light {
private:
	sptr<LIGHTS> mLights{};

	size_t mCrntLightCount{};
	LIGHTS* mCBMappedLights{};
	ComPtr<ID3D12Resource> mCBLights{};

	std::unordered_map<std::string, const LIGHT*> mLightModels;

public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Constructor ] /////

	Light();
	~Light();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Getter ] /////

	const LIGHT* GetLightModel(const std::string& modelName);
	LIGHT* GetLight(int index) { return &mLights->mLights[index]; }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Setter ] /////

private:
	void SetSunlight();

public:
	void SetGlobalLight(GlobalLight globalLight);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Others ] /////
	
	/* DirectX */
	void CreateShaderVariables();
	void UpdateShaderVariables();
	void ReleaseShaderVariables();

	/* Build */
private:
	void LoadLightModels();
	void LoadLightObjects(FILE* file);

public:
	void BuildLights(FILE* file);

	/* Others*/
	LIGHT_RANGE AlloLight(size_t count);

	void InsertLight(std::string name, const LIGHT* light) { mLightModels.insert(std::make_pair(name, light)); }
};