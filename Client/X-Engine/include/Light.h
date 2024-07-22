#pragma once

#pragma region ClassForwardDecl
class ModelObjectMesh;
class GameObject;
#pragma endregion


#pragma region EnumClass
// must be matched with Light.hlsl LightType
enum class LightType {
	Spot        = 0,
	Directional = 1,
	Point       = 2
};
#pragma endregion


#pragma region Class
class Light {
private:
	sptr<SceneLight>		mLights{};			// all lights in scene
	sptr<SceneLoadLight>	mLoadLights{};		// all load lights in scene
	size_t					mCurrLightCnt{};	// count of allocated light in scene
	
	BoundingSphere			mSceneBounds{};		// 시야 입체 빛은 해당 입체 부분에만 적용된다.
	Matrix					mMtxLightView{};
	Matrix					mMtxLightProj{};
	Matrix					mMtxShadow{};

	std::unordered_map<std::string, const LightLoadInfo*> mLightModels{};	// 하나의 조명 모델에 대해 여러 조명을 동적으로 생성 가능한 모델 목록
	std::set<std::string> mLightModelNames{};

public:
	Light();
	virtual ~Light();

	// [modelName]에 따른 LightInfo을 반환한다.
	const LightLoadInfo* GetLightModel(const std::string& modelName) const;

	// [index]에 따른 LightInfo를 반환한다.
	LightInfo* GetLight(int index) const { return &mLights->Lights[index]; }
	// 전체 조명을 반환한다.
	rsptr<SceneLight> GetSceneLights() const { return mLights; }
	UINT GetLightCount() const { return static_cast<UINT>(mLights->Lights.size()); }

	const Matrix& GetLightViewMtx() const { return mMtxLightView; }
	const Matrix& GetLightProjMtx() const { return mMtxLightProj; }
	const Matrix& GetShadowMtx() const { return mMtxShadow; }

public:
	// 새로운 조명 모델을 삽입한다.
	void InsertLightModel(const std::string& name, const LightLoadInfo* light) { mLightModels.insert(std::make_pair(name, light)); }

	void SetSceneBounds(float boundRadius);
	void BuildLights(std::ifstream& file);
	void BuildLights();

	void Update();
	void UpdateShaderVars(int index);

	// 조명의 볼륨 메쉬를 렌더링한다.
	void Render();

	// 조명을 Sunlight로 설정한다.
	void SetSunlight();

private:

	// 조명 모델 폴더에서 조명 모델들을 불러온다.
	void LoadLightModels();
	// [file]에서 조명 모델을 불러오고 할당한다.
	void LoadLightObjects(std::ifstream& file);
};
#pragma endregion