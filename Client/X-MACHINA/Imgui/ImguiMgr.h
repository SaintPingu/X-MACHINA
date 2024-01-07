#pragma once
#include "../include/Singleton.h"

/*

* ReadMe
	2024-01-07-SUN (장재문) 
	
* Imgui 사용

	Dear Imgui Demo 에 있는 모든 코드는 Imgui_demo.cpp 에 있습니다.
	Imgui_demo.cpp 파일 가셔서 사용 코드 복붙하는 식으로 쓰면 됩니다. 
	그거 가져다가 커스텀 하세요. 저는 원하는 기능을 Ctrl-F 를 이용해서 대충 이름 찾아서 썼어요. 
	

* Imgui Tool 관련 링크  
	
	현재 1. Imgui 만 추가했습니다. 
	2,3 은 필요하실 때 클라이언트가 알잘딱!
	아디오스!~
	
	1. Imgui									: https://github.com/ocornut/imgui
	2. Imgui Texture							: https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
	3. Imgui Guizmo, Sequencer, GraphEditor     : https://github.com/CedricGuillemet/ImGuizmo

*/

#pragma region Define
#define imgui ImguiMgr::Inst()
#pragma endregion

#pragma region Class
class ImguiMgr : public Singleton<ImguiMgr> {
	friend class Singleton<ImguiMgr>;

private:
	ComPtr<ID3D12DescriptorHeap> mSrvDescHeap{};
	bool						 mIsShowDemo = true;

	std::vector<std::function<void(ImguiMgr&)>> mImguiFuncs; // 이거는 제가 쓰는건데 필요없으면 빼도 됩니다. 제가 등록해놓은 함수를 새로운 Imgui 윈도우창에 실행시키는 용도로 만든거에요.

public:
	ImguiMgr();
	virtual ~ImguiMgr();

public:
	bool Init();
#pragma region Render
	void RenderPrepare();	
	void Render();			
	void Present();
#pragma endregion

public:
	void SampleFunc();
	void RegisterImguiFunc(std::function<void(ImguiMgr&)> func);
	void ImGuiWindow(std::string Label, std::vector<std::function<void(ImguiMgr&)>> functions);
};
#pragma endregion
