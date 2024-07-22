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
	virtual void RenderBegin() override;

	virtual void RenderShadow() override;
	virtual void RenderDeferred() override;
	virtual void RenderCustomDepth() override;
	virtual void RenderForward() override;
	virtual void RenderUI() override;

	virtual void ApplyDynamicContext() override;

	void Update();
	virtual void Build() override;
	virtual void Release() override;

private:

	void Start();
	void UpdateObjects();
	void RenderObjects();

	void Test();
};
