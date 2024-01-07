#include "stdafx.h"
#include "ImguiMgr.h"

#include "ImguiCode/imgui.h"
#include "ImguiCode/imgui_impl_win32.h"
#include "ImguiCode/imgui_impl_dx12.h"

#include "../include/DXGIMgr.h"

ImguiMgr::ImguiMgr()
{
}

ImguiMgr::~ImguiMgr()
{
    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool ImguiMgr::Init()
{
    // Sample 제가 사용하는 방식은 이러합니다. 빼도됩니다.
    ImguiMgr::RegisterImguiFunc(&ImguiMgr::SampleFunc);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls    
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();       // 1. 다크 모드
    //ImGui::StyleColorsLight();    // 2. 라이트 모드
    //ImGui::StyleColorsClassic();  // 3. 클래식 모드 

     // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding            = 5.f;                              
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.3f, 0.3f, 0.3f, 0.8f);   
        style.FramePadding              = ImVec2(2.f, 2.f);                 
        style.FrameRounding             = 1.f;
    }

    // create descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(device.Get()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mSrvDescHeap))))
        assert(nullptr);

    bool bSuccess = ImGui_ImplWin32_Init(dxgi->GetHwnd());
    bSuccess = ImGui_ImplDX12_Init(device.Get(), 3, DXGI_FORMAT_R8G8B8A8_UNORM
        , mSrvDescHeap.Get(), mSrvDescHeap->GetCPUDescriptorHandleForHeapStart()
        , mSrvDescHeap.Get()->GetGPUDescriptorHandleForHeapStart());

    return true;
}

void ImguiMgr::RenderPrepare()
{    
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImguiMgr::Render()
{ 
    if (mIsShowDemo)
        ImGui::ShowDemoWindow(&mIsShowDemo);

#pragma region Sample Code 
    // TODO : 내가 만든 렌더링 함수 넣으시면 됩니다.. ( SceneOutliner , TransformUI 등등... )
    ImGui::Begin("Sample Imgui Window Hi X-MACHINA");
    ImGui::Text("Hello XMachina");
    //ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.f, 0.f, 1.f, 1.f), "Hello World!");
    ImGui::End();

    ImguiMgr::ImGuiWindow("Sample 2", mImguiFuncs);
#pragma endregion

}

void ImguiMgr::Present()
{
    // Rendering
    cmdList->SetDescriptorHeaps(1, mSrvDescHeap.GetAddressOf());
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList.Get());

    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

}

void ImguiMgr::SampleFunc()
{
    ImGui::Text("Hello XMachina");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.f, 0.f, 1.f, 1.f), "Hello World!");

}

void ImguiMgr::RegisterImguiFunc(std::function<void(ImguiMgr&)> func)
{
    mImguiFuncs.push_back(func);
}

void ImguiMgr::ImGuiWindow(std::string Label, std::vector<std::function<void(ImguiMgr&)>> functions)
{
    ImGui::Begin(Label.c_str());

    for (std::function<void(ImguiMgr&)> func : functions)
    {
        func(*this);
    }

    ImGui::End();
}

