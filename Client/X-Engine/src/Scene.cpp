#include "EnginePch.h"
#include "Scene.h"

#include "Timer.h"
#include "Light.h"
#include "DXGIMgr.h"
#include "ResourceMgr.h"
#include "FrameResource.h"
#include "Model.h"
#include "Object.h"
#include "TextMgr.h"
#include "Mesh.h"
#include "Texture.h"
#include "SkyBox.h"
#include "Ssao.h"

#include "Component/UI.h"
#include "Component/Camera.h"


Scene::Scene()
{
	mManager = std::make_shared<Object>();
}

void Scene::Update()
{
	mManager->Update();
	Canvas::I->Update();
}

void Scene::Build()
{
	constexpr float kDefaultSceneBoundRadius = 30.f;

	mLight = std::make_shared<Light>();
	mLight->SetSceneBounds(kDefaultSceneBoundRadius);
	mSkyBox = std::make_shared<SkyBox>();
	
	std::cout << "Load " + mName + " Scene...\n";
}

void Scene::Release()
{
	if (mLight) {
		mLight = nullptr;
	}
	//if (mSkyBox) {
	//	mSkyBox = nullptr;
	//}
	if (mManager) {
		mManager->RemainFirstComponent();
		mManager->ResetComponents();
	}
}

void Scene::RenderLights()
{
	if (mLight) {
		mLight->Render();
	}
}

void Scene::RenderForward()
{
	if (mRenderForwardCallback) {
		CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mRenderForwardCallback();
	}
}

void Scene::RenderUI()
{
	Canvas::I->Render();
}

void Scene::RenderText(RComPtr<struct ID2D1DeviceContext2> device)
{
	TextMgr::I->Render(device);
}


void Scene::UpdateShaderVars()
{
	UpdateMainPassCB();
#ifndef RENDER_FOR_SERVER
	UpdateShadowPassCB();
	UpdateSsaoCB();
#endif
	UpdateMaterialBuffer();
}

void Scene::UpdateMainPassCB()
{
	Matrix proj = MAIN_CAMERA->GetProjMtx();
	PassConstants passCB;
	passCB.MtxView = MAIN_CAMERA->GetViewMtx().Transpose();
	passCB.MtxProj = MAIN_CAMERA->GetProjMtx().Transpose();
	passCB.MtxInvProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	passCB.MtxShadow = mLight->GetShadowMtx().Transpose();
	passCB.MtxNoLagView = MAIN_CAMERA->GetNoLagViewtx().Transpose();
	passCB.CameraPos = MAIN_CAMERA->GetPosition();
	passCB.CameraRight = MAIN_CAMERA->GetRight();
	passCB.CameraUp = MAIN_CAMERA->GetUp();
	passCB.DeltaTime = DeltaTime();
	passCB.TotalTime = Timer::I->GetTotalTime();
	passCB.FrameBufferWidth = DXGIMgr::I->GetWindowWidth();
	passCB.FrameBufferHeight = DXGIMgr::I->GetWindowHeight();
	passCB.SkyBoxIndex = (mSkyBox) ? mSkyBox->GetTexture()->GetSrvIdx() : 0;
	passCB.DefaultDsIndex = RESOURCE<Texture>("DefaultDepthStencil")->GetSrvIdx();
	passCB.ShadowDsIndex = RESOURCE<Texture>("ShadowDepthStencil")->GetSrvIdx();
	passCB.CustomDsIndex = RESOURCE<Texture>("CustomDepthStencil")->GetSrvIdx();
	passCB.RT0G_PositionIndex = RESOURCE<Texture>("PositionTarget")->GetSrvIdx();
	passCB.RT1G_NormalIndex = RESOURCE<Texture>("NormalTarget")->GetSrvIdx();
	passCB.RT2G_DiffuseIndex = RESOURCE<Texture>("DiffuseTarget")->GetSrvIdx();
	passCB.RT3G_EmissiveIndex = RESOURCE<Texture>("EmissiveTarget")->GetSrvIdx();
	passCB.RT4G_MetallicSmoothnessIndex = RESOURCE<Texture>("MetallicSmoothnessTarget")->GetSrvIdx();
	passCB.RT5G_OcclusionIndex = RESOURCE<Texture>("OcclusionTarget")->GetSrvIdx();
	passCB.RT0L_DiffuseIndex = RESOURCE<Texture>("DiffuseAlbedoTarget")->GetSrvIdx();
	passCB.RT1L_SpecularIndex = RESOURCE<Texture>("SpecularAlbedoTarget")->GetSrvIdx();
	passCB.RT2L_AmbientIndex = RESOURCE<Texture>("AmbientTarget")->GetSrvIdx();
	passCB.RT0S_SsaoIndex = RESOURCE<Texture>("SSAOTarget_0")->GetSrvIdx();
	passCB.RT0O_OffScreenIndex = RESOURCE<Texture>("OffScreenTarget")->GetSrvIdx();
	passCB.RT6G_OutlineIndex = RESOURCE<Texture>("OutlineTarget")->GetSrvIdx();
	passCB.BloomIndex = RESOURCE<Texture>("BloomTarget")->GetSrvIdx();
	passCB.LiveObjectDissolveIndex = RESOURCE<Texture>("LiveObjectDissolve")->GetSrvIdx();
	passCB.BuildingDissolveIndex = RESOURCE<Texture>("Dissolve_01_05")->GetSrvIdx();

	passCB.LightCount = mLight->GetLightCount();
	passCB.GlobalAmbient = Vec4(0.1f, 0.1f, 0.1f, 1.f);
	passCB.FilterOption = DXGIMgr::I->GetFilterOption();
	passCB.ShadowIntensity = 0.1f;
	passCB.FogColor = Colors::Gray;
	memcpy(&passCB.Lights, mLight->GetSceneLights().get()->Lights.data(), sizeof(passCB.Lights));

	FRAME_RESOURCE_MGR->CopyData(0, passCB);
}

void Scene::UpdateShadowPassCB()
{
	PassConstants passCB;
	passCB.MtxView = mLight->GetLightViewMtx().Transpose();
	passCB.MtxProj = mLight->GetLightProjMtx().Transpose();
	passCB.LiveObjectDissolveIndex = RESOURCE<Texture>("LiveObjectDissolve")->GetSrvIdx();

	FRAME_RESOURCE_MGR->CopyData(1, passCB);
}

void Scene::UpdateSsaoCB()
{
	SsaoConstants ssaoCB;

	Matrix mtxProj = MAIN_CAMERA->GetProjMtx();
	Matrix mtxTex = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f };

	ssaoCB.MtxInvProj = MAIN_CAMERA->GetProjMtx().Invert().Transpose();
	ssaoCB.MtxProjTex = (mtxProj * mtxTex).Transpose();
	DXGIMgr::I->GetSsao()->GetOffsetVectors(ssaoCB.OffsetVectors);

	// for Blur 
	auto blurWeights = Filter::CalcGaussWeights(2.5f);
	ssaoCB.BlurWeights[0] = Vec4(&blurWeights[0]);
	ssaoCB.BlurWeights[1] = Vec4(&blurWeights[4]);
	ssaoCB.BlurWeights[2] = Vec4(&blurWeights[8]);

	auto ssaoTarget = RESOURCE<Texture>("SSAOTarget_0");
	ssaoCB.InvRenderTargetSize = Vec2{ 1.f / ssaoTarget->GetWidth(), 1.f / ssaoTarget->GetHeight() };

	// coordinates given in view space.
	ssaoCB.OcclusionRadius = 0.5f;
	ssaoCB.OcclusionFadeStart = 0.2f;
	ssaoCB.OcclusionFadeEnd = 1.0f;
	ssaoCB.SurfaceEpsilon = 0.05f;
	ssaoCB.AccessContrast = 12;

	ssaoCB.RandomVectorIndex = RESOURCE<Texture>("RandomVector")->GetSrvIdx();

	FRAME_RESOURCE_MGR->CopyData(ssaoCB);
}

void Scene::UpdateMaterialBuffer()
{
	ResourceMgr::I->ProcessFunc<MasterModel>(
		[](sptr<MasterModel> model) {
			model->GetMesh()->UpdateMaterialBuffer();
		});
}