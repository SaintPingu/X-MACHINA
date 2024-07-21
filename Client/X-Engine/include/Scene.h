#pragma once
class Light;
class SkyBox;

enum class RenderType : UINT8 {
	Shadow,
	Forward,
	Deferred,
	CustomDepth,
	DynamicEnvironmentMapping,
};

class Scene abstract {
public:
	sptr<Light> mLight{};
	sptr<SkyBox> mSkyBox{};

public:
	Scene();

public:
	virtual void Init() {}

	virtual void Build();
	virtual void RenderBegin() abstract;

	virtual void RenderShadow() abstract;
	virtual void RenderDeferred() abstract;
	virtual void RenderLights();
	virtual void RenderCustomDepth() abstract;
	virtual void RenderForward() abstract;
	virtual void RenderUI() abstract;
	virtual void RenderText(RComPtr<struct ID2D1DeviceContext2> device);

	virtual void ApplyDynamicContext() abstract;

	virtual void UpdateShaderVars();
	virtual void UpdateMainPassCB();
	virtual void UpdateShadowPassCB();
	virtual void UpdateSsaoCB();
	virtual void UpdateMaterialBuffer();

};