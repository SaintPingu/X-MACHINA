#pragma once
class Light;
class SkyBox;
class Object;

enum class RenderType : UINT8 {
	Shadow,
	Forward,
	Deferred,
	CustomDepth,
	DynamicEnvironmentMapping,
};

enum class SceneType {
	Lobby,
	Battle,
	Loading
};

class Scene abstract {
private:
	std::function<void()> mRenderForwardCallback{};

protected:
	sptr<Object> mManager{};
	sptr<Light> mLight{};
	sptr<SkyBox> mSkyBox{};

public:
	Scene();

public:
	Light* GetLight() const { return mLight.get(); }

public:
	virtual void Init() {}
	virtual void Update() {}

	virtual void Build();
	virtual void Release();
	virtual void RenderBegin() {};

	virtual void RenderShadow() {};
	virtual void RenderDeferred() {};
	virtual void RenderLights();
	virtual void RenderCustomDepth() {};

	virtual void RenderForward();

	virtual void RenderUI();
	virtual void RenderText(RComPtr<struct ID2D1DeviceContext2> device);

	virtual void ApplyDynamicContext() {};
	virtual void RenderEnd() {}

	virtual void UpdateShaderVars();
	virtual void UpdateMainPassCB();
	virtual void UpdateShadowPassCB();
	virtual void UpdateSsaoCB();
	virtual void UpdateMaterialBuffer();

public:
	Object* GetManager() const { return mManager.get(); }

	void SetRenderForwardCallback(const std::function<void()>& callback) { mRenderForwardCallback = callback; }
};