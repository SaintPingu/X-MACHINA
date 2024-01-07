#pragma once
#include "../include/Singleton.h"

/*

* ReadMe
	2024-01-07-SUN (���繮) 
	
* Imgui ���

	Dear Imgui Demo �� �ִ� ��� �ڵ�� Imgui_demo.cpp �� �ֽ��ϴ�.
	Imgui_demo.cpp ���� ���ż� ��� �ڵ� �����ϴ� ������ ���� �˴ϴ�. 
	�װ� �����ٰ� Ŀ���� �ϼ���. ���� ���ϴ� ����� Ctrl-F �� �̿��ؼ� ���� �̸� ã�Ƽ� ����. 
	

* Imgui Tool ���� ��ũ  
	
	���� 1. Imgui �� �߰��߽��ϴ�. 
	2,3 �� �ʿ��Ͻ� �� Ŭ���̾�Ʈ�� ���ߵ�!
	�Ƶ����!~
	
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

	std::vector<std::function<void(ImguiMgr&)>> mImguiFuncs; // �̰Ŵ� ���� ���°ǵ� �ʿ������ ���� �˴ϴ�. ���� ����س��� �Լ��� ���ο� Imgui ������â�� �����Ű�� �뵵�� ����ſ���.

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
