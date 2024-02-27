#include "stdafx.h"
#include "ResourceMgr.h"

#include "FileIO.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

void ResourceMgr::Init()
{
	LoadTextures();
	LoadShaders();
	LoadRectangleMesh();
}

void ResourceMgr::Clear()
{
	for (auto& resource : mResources) {
		resource.clear();
	}
}

sptr<Texture> ResourceMgr::CreateTexture(const std::string& name, UINT width, UINT height, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS resourcecFlags, D3D12_RESOURCE_STATES resourceStates, Vec4 clearColor)
{
	sptr<Texture> texture = std::make_shared<Texture>();
	texture->Create(width, height, dxgiFormat, resourcecFlags, resourceStates, clearColor);
	Add<Texture>(name, texture);
	return texture;
}

sptr<Texture> ResourceMgr::CreateTexture(const std::string& name, ComPtr<ID3D12Resource> resource)
{
	sptr<Texture> texture = std::make_shared<Texture>();
	texture->Create(resource);
	Add<Texture>(name, texture);
	return texture;
}

sptr<ModelObjectMesh> ResourceMgr::LoadRectangleMesh()
{
	sptr<ModelObjectMesh> findMesh = Get<ModelObjectMesh>("Rect");
	if (findMesh)
		return findMesh;

	sptr<ModelObjectMesh> mesh = std::make_shared<ModelObjectMesh>();
	mesh->CreateRectangleMesh();
	Add<ModelObjectMesh>("Rect", mesh);
	return mesh;
}

void ResourceMgr::LoadTextures()
{
	FileIO::LoadTextures("Import/Textures/");
	FileIO::LoadTextures("Import/UI/");
}

void ResourceMgr::LoadShaders()
{
// DeferredShader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Global
	{
		ShaderInfo info = {
			ShaderType::Deferred,
		};

		ShaderPath path = {
			 L"VShader_Standard.cso",
			 L"PShader_Deferred.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Global", shader);
	}
#pragma endregion
#pragma region ObjectInst
	{
		ShaderInfo info = {
			ShaderType::Deferred,
		};

		ShaderPath path = {
			 L"VShader_StandardInstance.cso",
			 L"PShader_Deferred.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("ObjectInst", shader);
	}
#pragma endregion
#pragma region ColorInst
	{
		ShaderInfo info = {
			ShaderType::Deferred,
			RasterizerType::Cull_Back,
			DepthStencilType::Less,
			BlendType::Default,
			InputLayoutType::ColorInst
		};

		ShaderPath path = {
			 L"VShader_Instance.cso",
			 L"PShader_Instance.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("ColorInst", shader);
	}
#pragma endregion
#pragma region SkinMesh
	{
		ShaderInfo info = {
			ShaderType::Deferred,
		};

		ShaderPath path = {
			 L"VShader_SkinnedMesh.cso",
			 L"PShader_Deferred.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("SkinMesh", shader);
	}
#pragma endregion
#pragma region Terrain
	{
		ShaderInfo info = {
			ShaderType::Deferred,
		};

		ShaderPath path = {
			 L"VShader_Terrain.cso",
			 L"PShader_Terrain.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Terrain", shader);
	}
#pragma endregion

// ForwardShader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Water
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::Less_No_Write,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 L"VShader_Water.cso",
			 L"PShader_Water.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Water", shader);
	}
#pragma endregion
#pragma region Billboard
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::Less,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 L"VShader_Billboard.cso",
			 L"PShader_Billboard.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Billboard", shader);
	}
#pragma endregion
#pragma region Sprite
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::Less,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 L"VShader_Sprite.cso",
			 L"PShader_Billboard.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Sprite", shader);
	}
#pragma endregion
#pragma region Final
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 L"VShader_Tex.cso",
			 L"PShader_Final.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Final", shader);
	}
#pragma endregion
#pragma region Canvas
	{
		ShaderInfo info = {
			ShaderType::LDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 L"VShader_Canvas.cso",
			 L"PShader_Canvas.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Canvas", shader);
	}
#pragma endregion
#pragma region Wire
	{
		ShaderInfo info = {
			ShaderType::LDR,
			RasterizerType::WireFrame,
			DepthStencilType::Less,
			BlendType::Default,
			InputLayoutType::Wire,
			D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_LINELIST
		};

		ShaderPath path = {
			 L"VShader_Wired.cso",
			 L"PShader_Wired.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Wire", shader);
	}
#pragma endregion
#pragma region OffScreen
	{
		ShaderInfo info = {
			ShaderType::LDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 L"VShader_Tex.cso",
			 L"PShader_OffScreen.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("OffScreen", shader);
	}
#pragma endregion
#pragma region Lighting
	{
		ShaderInfo info = {
			ShaderType::Lighting,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 L"VShader_Tex.cso",
			 L"PShader_DirLighting.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("DirLighting", shader);
	}
#pragma endregion
#pragma region Transparent
	{
		ShaderInfo info = {
			ShaderType::LDR,
			RasterizerType::Cull_Back,
			DepthStencilType::Less_No_Write,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 L"VShader_Standard.cso",
			 L"PShader_Standard.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Transparent", shader);
	}
#pragma endregion
#pragma region SkyBox
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_None,
			DepthStencilType::Less_Equal,
		};

		ShaderPath path = {
			 L"VShader_Skybox.cso",
			 L"PShader_Skybox.cso",
			 L""
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("SkyBox", shader);
	}
#pragma endregion

// ComputeShader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region HorzBlur
	{
		ShaderInfo info{};

		ShaderPath path = {
			 L"",
			 L"",
			 L"CShader_HorzBlur.cso"
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("HorzBlur", shader);
	}
#pragma endregion
#pragma region VertBlur
	{
		ShaderInfo info{};

		ShaderPath path = {
			 L"",
			 L"",
			 L"CShader_VertBlur.cso"
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("VertBlur", shader);
	}
#pragma endregion
#pragma region VertBlur
	{
		ShaderInfo info{};

		ShaderPath path = {
			 L"",
			 L"",
			 L"CShader_LUT.cso"
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("LUT", shader);
	}
#pragma endregion
}
