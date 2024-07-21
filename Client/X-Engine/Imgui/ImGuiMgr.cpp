#include "EnginePch.h"

#include "ImguiCode//imgui.h"
#include "ImguiCode/imgui_impl_win32.h"
#include "ImguiCode/imgui_impl_dx12.h"
#include "ImguiCode/imgui_internal.h"

#include "ImGuiMgr.h"
#include "DXGIMgr.h"
#include "BattleScene.h"
#include "Object.h"
#include "Component/ParticleSystem.h"
#include <iostream>

ImGuiMgr::ImGuiMgr()
{
}

ImGuiMgr::~ImGuiMgr()
{
    DestroyImGui();
}

bool ImGuiMgr::Init()
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
        style.WindowRounding = 8.f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.8f);
        style.FramePadding = ImVec2(2.f, 2.f);
        style.FrameRounding = 1.f;
    }

    // create descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(DEVICE->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mSrvDescHeap))))
        assert(nullptr);

    bool bSuccess = ImGui_ImplWin32_Init(DXGIMgr::I->GetHwnd());
    bSuccess = ImGui_ImplDX12_Init(DEVICE.Get(), 3, DXGI_FORMAT_R8G8B8A8_UNORM
        , mSrvDescHeap.Get(), mSrvDescHeap->GetCPUDescriptorHandleForHeapStart()
        , mSrvDescHeap.Get()->GetGPUDescriptorHandleForHeapStart());

    return true;
}

void ImGuiMgr::Render_Prepare()
{
    if (!mIsOn)
        return;

    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiMgr::Update()
{
    if (!mIsOn)
        return;

    if (mIsShowDemo)
        ImGui::ShowDemoWindow(&mIsShowDemo);

    mHierachyFunc.Execute(nullptr);
    mInspector.Execute(mHierachyFunc.GetSelectedObject());
    ImGui::Begin("test", NULL);
    mIsFocused = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();
    ImGui::End();
}

void ImGuiMgr::Render()
{
    if (!mIsOn)
        return;

    // Rendering
    CMD_LIST->SetDescriptorHeaps(1, mSrvDescHeap.GetAddressOf());

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CMD_LIST.Get());

    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void ImGuiMgr::DestroyImGui()
{
    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiFunc::Execute(GameObject* selectedObject)
{
	if (!selectedObject)
		selectedObject = mSelectedObject;
	else
		mSelectedObject = selectedObject;
}

void ImGuiHierachy::Execute(GameObject* selectedObject)
{
	base::Execute(selectedObject);

	ImGui::Begin(mLabel.c_str(), nullptr, mNoMoveWindow ? ImGuiWindowFlags_None : ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	
	if (!selectedObject)
		selectedObject = mSelectedObject;
	else
		mSelectedObject = selectedObject;

    size_t entityID = -1;
    for (const auto& object : BattleScene::I->GetAllObjects()) {
        DrawNode(object.get(), entityID);
    }

	ImGui::End();
}


void ImGuiMgr::FocusOff()
{
    ImGui::FocusWindow(NULL);
    mIsFocused = false;
}


void ImGuiHierachy::DrawNode(GameObject* node, size_t& entityID)
{
    entityID++;

    std::string name = node->GetName() + "##";
    name += std::to_string(entityID);

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (!node->mChild) nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (node == mSelectedObject) nodeFlags |= ImGuiTreeNodeFlags_Selected;

    if (node->mSibling) {
        DrawNode(node->mSibling->GetObj<GameObject>(), entityID);
    }

    if (ImGui::TreeNodeEx(name.c_str(), nodeFlags)) {
        if (ImGui::IsItemClicked()) {
            mSelectedObject = node;
        }

        if (node->mChild) {
            DrawNode(node->mChild->GetObj<GameObject>(), entityID);
        }

        ImGui::TreePop();
    }
}

void ImGuiParticleSystem::Execute(GameObject* selectedObject)
{
	base::Execute(selectedObject);
//
//	if (!mSelectedObject)
//		return;
//
//	if (auto& ps = mSelectedObject->GetComponent<ParticleSystem>()) {
//		if (ImGui::CollapsingHeader(mLabel.c_str())) {
//			auto& pscd = ps->GetPSCD();
//
//			ImGui::SetNextItemWidth(220);
//			ImGui::InputFloat("Duration", &pscd.Duration);
//			ImGui::Checkbox("Looping", &pscd.Looping);
//			ImGui::Checkbox("Prewarm", &pscd.Prewarm);
//			ImGui::SetNextItemWidth(220);
//			ImGui::InputFloat("Start Delay", &pscd.StartDelay);
//
//			const char* options[] = { "Constant", "Random Between Two Constants" };
//
//#pragma region StartLifetime
//			{
//				static int currentItem = 0;
//				if (ImGui::BeginCombo("##HiddenCombo", options[currentItem], ImGuiComboFlags_NoPreview)) {
//					for (int i = 0; i < IM_ARRAYSIZE(options); i++) {
//						const bool isSelected = (currentItem == i);
//						if (ImGui::Selectable(options[i], isSelected))
//							currentItem = i;
//
//						if (isSelected)
//							ImGui::SetItemDefaultFocus();
//					}
//					ImGui::EndCombo();
//				}
//
//				ImGui::SameLine();
//				ImGui::SetNextItemWidth(200);
//
//				static float x = pscd.StartLifeTime.x;
//				static float vec2f[2] = { pscd.StartLifeTime.x, pscd.StartLifeTime.y };
//				switch (currentItem)
//				{
//				case 0:
//					ImGui::InputFloat("Start Lifetime", &x);
//					pscd.StartLifeTime = x;
//					break;
//				case 1:
//					ImGui::InputFloat2("Start Lifetime", vec2f);
//					pscd.StartLifeTime = Vec2{ vec2f[0], vec2f[1] };
//					break;
//				default:
//					break;
//				}
//			}
//#pragma endregion
//
//#pragma region StartSpeed
//			{
//				static int currentItem2 = 0;
//				if (ImGui::BeginCombo("##HiddenCombo2", options[currentItem2], ImGuiComboFlags_NoPreview)) {
//					for (int i = 0; i < IM_ARRAYSIZE(options); i++) {
//						const bool isSelected = (currentItem2 == i);
//						if (ImGui::Selectable(options[i], isSelected))
//							currentItem2 = i;
//
//						if (isSelected)
//							ImGui::SetItemDefaultFocus();
//					}
//					ImGui::EndCombo();
//				}
//
//				ImGui::SameLine();
//				ImGui::SetNextItemWidth(200);
//
//				static float x = pscd.StartSpeed.x;
//				static float vec2f[2] = { pscd.StartSpeed.x, pscd.StartSpeed.y };
//				switch (currentItem2)
//				{
//				case 0:
//					ImGui::InputFloat("Start Speed", &x);
//					pscd.StartSpeed = x;
//					break;
//				case 1:
//					ImGui::InputFloat2("Start Speed", vec2f);
//					pscd.StartSpeed = Vec2{ vec2f[0], vec2f[1] };
//					break;
//				default:
//					break;
//				}
//			}
//
//            static bool isStartSize3D = false;
//            ImGui::Checkbox("3D Start Size", &isStartSize3D);
//
//            if (isStartSize3D) {
//                pscd.StartSize3D.w = 1.f;
//                static float vec3f[3] = { pscd.StartSize.x, pscd.StartSize.y, pscd.StartSize.y };
//                ImGui::InputFloat3("Start Size", vec3f);
//                pscd.StartSize3D = Vec4{ vec3f[0], vec3f[1], vec3f[2], 1.f };
//            }
//            else {
//                pscd.StartSize3D.w = 0.f;
//                static int currentItem3 = 0;
//                if (ImGui::BeginCombo("##HiddenCombo3", options[currentItem3], ImGuiComboFlags_NoPreview)) {
//                    for (int i = 0; i < IM_ARRAYSIZE(options); i++) {
//                        const bool isSelected = (currentItem3 == i);
//                        if (ImGui::Selectable(options[i], isSelected))
//                            currentItem3 = i;
//
//                        if (isSelected)
//                            ImGui::SetItemDefaultFocus();
//                    }
//                    ImGui::EndCombo();
//                }
//
//                ImGui::SameLine();
//                ImGui::SetNextItemWidth(200);
//
//                static float x = pscd.StartSize.x;
//                static float vecs2f[2] = { pscd.StartSize.x, pscd.StartSize.y };
//                switch (currentItem3)
//                {
//                case 0:
//                    ImGui::InputFloat("Start Size", &x);
//                    pscd.StartSize = x;
//                    break;
//                case 1:
//                    ImGui::InputFloat2("Start Size", vecs2f);
//                    pscd.StartSize = Vec2{ vecs2f[0], vecs2f[1] };
//                    break;
//                default:
//                    break;
//                }
//            }
//
//#pragma endregion
//
//			ImVec2 windowSize = ImGui::GetContentRegionAvail();
//			ImVec2 buttonSize(50, 25);
//			ImVec2 buttonPos(windowSize.x * 0.5f - buttonSize.x * 1.5f, windowSize.y + 120.f);
//			ImGui::SetCursorPos(buttonPos);
//
//			const char* playButtonText = pscd.IsStop ? "Play" : "Pause";
//			if (ImGui::Button(playButtonText, buttonSize)) ps->Play();
//			ImGui::SameLine();
//			if (ImGui::Button("Restart", buttonSize)) int a = 3/* ps->Restart()*/;
//			ImGui::SameLine();
//			if (ImGui::Button("Stop", buttonSize)) ps->Stop();
//
//		}
//	}
}

void ImGuiInspector::Execute(GameObject* selectedObject)
{
	base::Execute(selectedObject);

	ImGui::Begin(mLabel.c_str(), nullptr, mNoMoveWindow ? ImGuiWindowFlags_None : ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	
	mTransformFunc.Execute(mSelectedObject);
	mParticleSystemFunc.Execute(mSelectedObject);

	ImGui::End();
}

void ImGuiTransform::Execute(GameObject* selectedObject)
{
	base::Execute(selectedObject);

	if (!mSelectedObject)
		return;

	if (ImGui::CollapsingHeader(mLabel.c_str())) {
		float transformVec3[3] = { mSelectedObject->GetLocalPosition().x, mSelectedObject->GetLocalPosition().y , mSelectedObject->GetLocalPosition().z };
		ImGui::InputFloat3("Position", transformVec3);
		mSelectedObject->SetPosition(transformVec3[0], transformVec3[1], transformVec3[2]);

		float rotationVec3[3] = { mSelectedObject->GetLocalRotation().x, mSelectedObject->GetLocalRotation().y , mSelectedObject->GetLocalRotation().z };
		ImGui::InputFloat3("Rotation", rotationVec3);
	}
}
