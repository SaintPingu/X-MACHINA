#include "stdafx.h"

#include "ImguiCode//imgui.h"
#include "ImguiCode/imgui_impl_win32.h"
#include "ImguiCode/imgui_impl_dx12.h"
#include "ImguiCode/imgui_internal.h"

#include "ImGuiManager.h"
#include "DXGIMgr.h"
#include "Scene.h"
#include "Object.h"
#include "Component/ParticleSystem.h"



ImGuiManager::ImGuiManager()
{
}

ImGuiManager::~ImGuiManager()
{
    DestroyImGui();
    Destroy();
}

bool ImGuiManager::Init()
{

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls    
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    //ImGui::StyleColorsLight();
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding            = 8.f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.8f);
        style.FramePadding              = ImVec2(2.f, 2.f);
        style.FrameRounding             = 1.f;
    }

    // create descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mSrvDescHeap))))
        assert(nullptr);

    bool bSuccess = ImGui_ImplWin32_Init(dxgi->GetHwnd());
    bSuccess = ImGui_ImplDX12_Init(device.Get(), 3, DXGI_FORMAT_R8G8B8A8_UNORM
        , mSrvDescHeap.Get(), mSrvDescHeap->GetCPUDescriptorHandleForHeapStart()
        , mSrvDescHeap.Get()->GetGPUDescriptorHandleForHeapStart());

    return true;
}

void ImGuiManager::Render_Prepare()
{    
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::Update()
{
    if (mIsShowDemo)
        ImGui::ShowDemoWindow(&mIsShowDemo);

    ImGui::Begin("test");

    ImGui::End();
}

void ImGuiManager::Render()
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

void ImGuiManager::DestroyImGui()
{
    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}


void ImGuiManager::ImGuiWindow(std::string Label, std::function<void(ImGuiManager&)> function)
{
    ImGui::Begin(Label.c_str());

    function(*this);

    ImGui::End();
}






