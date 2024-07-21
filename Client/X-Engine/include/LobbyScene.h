#pragma once

#include "Scene.h"

class UI;
class GameObject;

class LobbyScene : public Singleton<LobbyScene>, public Scene {
	friend Singleton;

public:
	sptr<UI> mCursor{};
	std::vector<sptr<GameObject>> mObjects{};

public:
	void Init();
	void Update();

public:
	virtual void RenderBegin() override;

	virtual void RenderShadow() override;
	virtual void RenderDeferred() override;
	virtual void RenderLights() override;
	virtual void RenderCustomDepth() override;
	virtual void RenderForward() override;
	virtual void RenderUI() override;
	virtual void RenderText(RComPtr<ID2D1DeviceContext2> device) override;

	virtual void ApplyDynamicContext() override;
};
