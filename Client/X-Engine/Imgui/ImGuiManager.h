#pragma once
#include "Singleton.h"

#define IMGUI_MGR ImGuiManager::Inst()
#define IMGUI_COUT(logString) ImGuiManager::Inst()->Text(logString)
#define IMGUI_LOG(consoleStrID, logMsg) ImGuiManager::Inst()->AddLog(consoleStrID, logMsg)
#define IMGUI_MAIN_LOG(logMsg) ImGuiManager::Inst()->AddMainLog(logMsg)

#define imgui ImGuiManager::Inst()
class ImGuiManager : public Singleton<ImGuiManager>
{
	friend Singleton;

private:
	ComPtr<ID3D12DescriptorHeap>						mSrvDescHeap{};
	bool												mIsShowDemo = true;

public:
	ImGuiManager();
	~ImGuiManager();

public:
	bool Init();
	void Render_Prepare();
	void Update();
	void Render();

	void DestroyImGui();
	void ImGuiWindow(std::string Label, std::function<void(ImGuiManager&)> function);

};
