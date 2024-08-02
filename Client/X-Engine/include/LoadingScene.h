#pragma once


#pragma region Include
#include "Scene.h"
#pragma endregion

class UI;


class LoadingScene : public Singleton<LoadingScene>, public Scene {
	friend Singleton;

private:
	UI*		mBackground{};
	Scene*	mNextScene{};

public:
	virtual void Build();
	virtual void Release();
	virtual void RenderBegin() override {}

	virtual void RenderShadow() override {}
	virtual void RenderDeferred() override {}
	virtual void RenderLights() override {}
	virtual void RenderCustomDepth() override {}
	virtual void RenderForward() override {}

	virtual void ApplyDynamicContext() override {}

	virtual void UpdateShaderVars() override {}

public:
	void SetNextScene(Scene* scene) { mNextScene = scene; }
};