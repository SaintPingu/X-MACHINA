#pragma once
#include "Singleton.h"

class GameObject;

class ImGuiFunc {
protected:
	GameObject* mSelectedObject{};
	std::string mLabel{};
	bool mNoMoveWindow = true;

public:
	ImGuiFunc(std::string label) : mLabel(label) {}
	virtual ~ImGuiFunc() = default;

public:
	virtual void Execute(GameObject* selectedObject);

	void SetNoMoveWindow(bool val) { mNoMoveWindow = val; }
	GameObject* GetSelectedObject() { return mSelectedObject; }
};


class ImGuiHierachy : public ImGuiFunc {
	using base = ImGuiFunc;

public:
	ImGuiHierachy() : ImGuiFunc("Hierachy") {}

public:
	virtual void Execute(GameObject* selectedObject) override;

private:
	void DrawNode(GameObject* node, size_t& entityID);
};


class ImGuiTransform : public ImGuiFunc {
	using base = ImGuiFunc;

public:
	ImGuiTransform() : ImGuiFunc("Transform") {}

public:
	virtual void Execute(GameObject* selectedObject) override;
};


class ImGuiParticleSystem : public ImGuiFunc {
	using base = ImGuiFunc;

public:
	ImGuiParticleSystem() : ImGuiFunc("ParticleSystem") {}

public:
	virtual void Execute(GameObject* selectedObject) override;
};


class ImGuiInspector : public ImGuiFunc {
	using base = ImGuiFunc;

private:
	ImGuiParticleSystem	mParticleSystemFunc;
	ImGuiTransform		mTransformFunc;

public:
	ImGuiInspector() : ImGuiFunc("Inspector") {}

public:
	virtual void Execute(GameObject* selectedObject) override;
};


class ImGuiMgr : public Singleton<ImGuiMgr>
{
	friend Singleton;

private:
	bool							mIsOn = false;
	
	ComPtr<ID3D12DescriptorHeap>	mSrvDescHeap{};
	bool							mIsShowDemo = true;

	ImGuiHierachy					mHierachyFunc;
	ImGuiInspector					mInspector;
	bool mIsFocused{};

public:
	ImGuiMgr();
	~ImGuiMgr();

public:
	void ToggleImGui() { mIsOn = !mIsOn; }
	bool Init();
	void Render_Prepare();
	void Update();
	void Render();
	void DestroyImGui();

	bool IsFocused() const { return mIsFocused ; }
	void FocusOff();
};

