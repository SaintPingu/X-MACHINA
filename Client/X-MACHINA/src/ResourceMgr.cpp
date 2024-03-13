#include "stdafx.h"
#include "ResourceMgr.h"

#include "FileIO.h"
#include "Texture.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "AnimationClip.h"
#include "AnimatorController.h"

void ResourceMgr::LoadResources()
{
	LoadTextures();
	LoadRectangleMesh();
	LoadPointMesh();
	LoadModels();
	LoadShaders();
	LoadAnimationClips();
	LoadAnimatorControllers();
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

sptr<ModelObjectMesh> ResourceMgr::LoadPointMesh()
{
	sptr<ModelObjectMesh> findMesh = Get<ModelObjectMesh>("Point");
	if (findMesh)
		return findMesh;

	sptr<ModelObjectMesh> mesh = std::make_shared<ModelObjectMesh>();
	mesh->CreatePointMesh();
	Add<ModelObjectMesh>("Point", mesh);
	return mesh;
}

void ResourceMgr::LoadTextures()
{
	FileIO::LoadTextures("Import/Textures/");
	FileIO::LoadTextures("Import/UI/");
	FileIO::LoadTextures("Import/Skybox/", D3DResource::TextureCube);
}

void ResourceMgr::LoadModels()
{
	const std::vector<std::string> binModelNames = { "tank_bullet", "sprite_explosion", };

	sptr<MasterModel> model;
	for (auto& name : binModelNames) {
		if (!mResources[static_cast<UINT8>(ResourceType::Model)].contains(name)) {
			model = FileIO::LoadGeometryFromFile("Import/Meshes/" + name + ".bin");
			if (name.substr(0, 6) == "sprite") {
				model->SetSprite();
			}
			Add<MasterModel>(name, model);
		}
	}
}

void ResourceMgr::LoadShaders()
{
// Shadow
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Shadow
	{
		ShaderInfo info = {
			ShaderType::Shadow,
			RasterizerType::DepthBias,
		};

		ShaderPath path = {
			 L"VShader_Shadow.cso",
			 L"PShader_Shadow.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Shadow", shader);
	}
#pragma endregion

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
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Global", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::Shadow,
			RasterizerType::DepthBias,
		};

		ShaderPath path = {
			 L"VShader_Shadow.cso",
			 L"PShader_Shadow.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Shadow_Global", shader);
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
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("SkinMesh", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::Shadow,
			RasterizerType::DepthBias,
		};

		ShaderPath path = {
			 L"VShader_Shadow_SkinnedMesh.cso",
			 L"PShader_Shadow.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Shadow_SkinMesh", shader);
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
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("DirLighting", shader);
	}

	{
		ShaderInfo info = {
			ShaderType::Lighting,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 L"VShader_SpotPointLighting.cso",
			 L"PShader_SpotPointLighting.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("SpotPointLighting", shader);
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
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("SkyBox", shader);
	}
#pragma endregion
#pragma region SSAO
	{
		ShaderInfo info = {
			ShaderType::Ssao,
			RasterizerType::Cull_None,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 L"VShader_Ssao.cso",
			 L"PShader_Ssao.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Ssao", shader);
	}
#pragma endregion
#pragma region SSAOBlur
	{
		ShaderInfo info = {
			ShaderType::Ssao,
			RasterizerType::Cull_None,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 L"VShader_Ssao.cso",
			 L"PShader_SsaoBlur.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("SsaoBlur", shader);
	}
#pragma endregion
#pragma region GraphicsParticle
	{
		ShaderInfo info{
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::Less_No_Write,
			BlendType::Alpha_Blend,
			InputLayoutType::Default,
			D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		};

		ShaderPath path = {
			 L"VShader_Particle.cso",
			 L"PShader_Particle.cso",
			 L"GShader_Particle.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("GraphicsParticle", shader);
	}
#pragma endregion
// ComputeShader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region HorzBlur
	{
		ShaderInfo info{
			ShaderType::Compute,
		};

		ShaderPath path = {
			 L"",
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
		ShaderInfo info{
			ShaderType::Compute,
		};

		ShaderPath path = {
			 L"",
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
		ShaderInfo info{
			ShaderType::Compute,
		};

		ShaderPath path = {
			 L"",
			 L"",
			 L"",
			 L"CShader_LUT.cso"
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("LUT", shader);
	}
#pragma endregion
#pragma region ComputeParticle
	{
		ShaderInfo info{
			ShaderType::Particle,
		};

		ShaderPath path = {
			 L"",
			 L"",
			 L"",
			 L"CShader_Particle.cso"
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("ComputeParticle", shader);
	}
#pragma endregion
}

void ResourceMgr::LoadAnimationClips()
{
	const std::string rootFolder = "Import/AnimationClips/";
	for (const auto& clipFolder : std::filesystem::directory_iterator(rootFolder)) {
		std::string clipFolderName = clipFolder.path().filename().string();

		for (const auto& file : std::filesystem::directory_iterator(rootFolder + clipFolderName + '/')) {
			std::string fileName = file.path().filename().string();
			sptr<AnimationClip> clip = FileIO::LoadAnimationClip(clipFolder.path().string() + '/' + fileName);

			FileIO::RemoveExtension(fileName);
			const std::string clipName = clipFolderName + '/' + fileName;
			res->Add<AnimationClip>(clipName, clip);
		}
	}
}

void ResourceMgr::LoadAnimatorControllers()
{
	const std::string rootFolder = "Import/AnimatorControllers/";
	for (const auto& file : std::filesystem::directory_iterator(rootFolder)) {
		const std::string fileName = file.path().filename().string();
		res->Add<AnimatorController>(FileIO::RemoveExtension(fileName), FileIO::LoadAnimatorController(rootFolder + fileName));
	}
}