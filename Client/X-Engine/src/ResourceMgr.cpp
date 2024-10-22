#include "EnginePch.h"
#include "ResourceMgr.h"

#include "FileIO.h"
#include "Texture.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "SoundMgr.h"
#include "AnimationClip.h"
#include "AnimatorController.h"
#include "Component/ParticleSystem.h"

void ResourceMgr::LoadResources()
{
	CreateParticleSystemCPUData();

	LoadTextures();
	LoadParticleSystemCPUData();
	LoadRectangleMesh();
	LoadPointMesh();
	LoadModels();
	LoadShaders();
	LoadAnimationClips();
	LoadAnimatorControllers();
	LoadSounds();
}

void ResourceMgr::Clear()
{
	for (auto& resource : mResources) {
		resource.clear();
	}
}

sptr<Texture> ResourceMgr::CreateTexture(const std::string& name, UINT width, UINT height, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS resourcecFlags, D3D12_RESOURCE_STATES resourceStates, Vec4 clearColor, D3DResource textureType)
{
	sptr<Texture> texture = std::make_shared<Texture>(textureType);
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

void ResourceMgr::CreateParticleSystemCPUData()
{
#pragma region MagicMissile
	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Explosion_Small";
		pscd.StartDelay = 0.05f;
		pscd.StartLifeTime = Vec2{ 0.3f, 0.4f };
		pscd.StartSpeed = 6.f;
		pscd.StartSize = 0.05f;
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 2.f, 2.f, 2.f, 1.f } });
		pscd.GravityModifier = 1.2f;
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.MaxParticles = 22;
		pscd.Emission.SetBurst(12);
		pscd.Emission.IsOn = true;
		pscd.Shape.SetSphere(0.01f, 1.f, 360.f, false);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 1.f, 1.f, 1.f }, Vec4{0.7f, 0.2f, 0.f, 1.0f } }, { 0.f, 1.f });
		pscd.Renderer.TextureName = "Explosion";
		pscd.Renderer.RenderMode = PSRenderMode::StretchedBillboard;
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Stretched_Blend;
		pscd.Renderer.LengthScale = 5.f;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Glow";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 0.1f };
		pscd.StartSpeed = 0.f;
		pscd.StartSize = 2.f;
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 2.f, 1.f, 0.6f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.MaxParticles = 11;
		pscd.Emission.SetBurst(1);
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 1.f, 1.f, 1.f }, Vec4{0.2f, 0.05f, 0.05f, 1.f } }, { 0.f, 1.f });
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Blend;
		pscd.Renderer.TextureName = "Explosion_Grow";
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Smoke_BigQuick";
		pscd.Duration = 0.5f;
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 0.3f };
		pscd.StartSpeed = 0.4f;
		pscd.StartSize = Vec2{ 1.2f, 1.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.MaxParticles = 13;
		pscd.Emission.SetBurst(1, 0.025f);
		pscd.Emission.SetBurst(1, 0.1f);
		pscd.Emission.SetBurst(1, 0.2f);
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		pscd.Shape.SetSphere(0.2f, 1.f, 360.f, false);
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 1.f, 1.f, 1.f }, Vec3{1.f, 1.f, 1.f } }, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({0.2f, 0.4f, 0.f}, {0.f, 0.5f, 1.f});
		pscd.RotationOverLifetime.Set(PSValOp::Constant, { 25.f });
		pscd.Renderer.BlendType = BlendType::Alpha_Blend;
		pscd.Renderer.TextureName = "Explosion_Smoke";
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Smoke_Building";
		pscd.Duration = 0.5f;
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 1.3f };
		pscd.StartSpeed = 0.3f;
		pscd.StartSize = Vec2{ 2.2f, 3.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 0.3f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.MaxParticles = 10;
		pscd.Emission.SetBurst(1);
		pscd.Emission.SetBurst(1, 0.1f);
		pscd.Emission.SetBurst(1, 0.2f);
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		pscd.Shape.SetSphere(0.2f, 1.f, 360.f, false);
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 1.f, 1.f, 1.f }, Vec3{1.f, 1.f, 1.f } }, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.f, 1.f, 0.f }, { 0.f, 0.5f, 1.f });
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, -1.f, 0.f, 0.f } }).SetParam(1.f);
		pscd.RotationOverLifetime.Set(PSValOp::Constant, { 50.f });
		pscd.Renderer.BlendType = BlendType::Alpha_Blend;
		pscd.Renderer.TextureName = "Explosion_Smoke";
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Dot_Sparkles_Mult";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 0.25f, 0.5f };
		pscd.StartSpeed = Vec2{ 1.f, 2.f };
		pscd.StartSize = Vec2{ 0.6f, 0.8f };
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.MaxParticles = 22;
		pscd.Emission.SetBurst(12);
		pscd.Shape.SetSphere(0.2f, 0.f, 360.f, false);
		pscd.Renderer.BlendType = BlendType::Multiply_Blend;
		pscd.Renderer.TextureName = "Explosion_SmallDot";
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Dot_Sparkles_Big";
		pscd.StartDelay = 0.05f;
		pscd.StartLifeTime = Vec2{ 0.2f, 0.35f };
		pscd.StartSpeed = Vec2{ 6.f, 8.f };
		pscd.StartSize = Vec2{ 0.16f, 0.24f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 3.f, 3.f, 3.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.GravityModifier = 4.f;
		pscd.MaxParticles = 21;
		pscd.Emission.SetBurst(8);
		pscd.Emission.SetBurst(3, 0.1f);
		pscd.Shape.SetSphere(0.3f, 0.f, 360.f, false);
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 1.f, 1.f, 1.f }, Vec3{ 1.f, 0.98f, 0.84f }, Vec3{1.f, 0.9f, 0.47f} }, { 0.f, 0.2f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({1.f, 1.f, 0.f}, {0.f, 0.4f, 1.f});
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.2f, 0.8f }, { 0.f, 1.f });
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Blend;
		pscd.Renderer.TextureName = "Explosion_Grow";
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Dot_Sparkles";
		pscd.StartDelay = 0.05f;
		pscd.StartLifeTime = Vec2{ 0.25f, 0.5f };
		pscd.StartSpeed = Vec2{ 1.f, 2.f };
		pscd.StartSize = Vec2{ 0.7f, 0.9f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.GravityModifier = 0.5f;
		pscd.MaxParticles = 40;
		pscd.Emission.SetBurst(24);
		pscd.Emission.SetBurst(6, 0.1f);
		pscd.Shape.SetSphere(0.1f, 1.f, 360.f, false);
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, 1.5f, 0.f, 0.f }, Vec4{ 0.f, 2.f, 0.f, 0.f }, Vec4{ 0.f, -2.f, 0.f, 0.f }}, {0.f, 0.5f, 0.1f}).SetParam(1.f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 1.f, 1.f, 1.f }, Vec4{ 0.7f, 0.1f, 0.f, 1.f } }, { 0.f, 1.f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		pscd.Renderer.TextureName = "Explosion_Dot";
		pscd.Renderer.RenderMode = PSRenderMode::StretchedBillboard;
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Explosion";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 0.13f };
		pscd.StartSize = Vec2{ 0.8f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 2.f, 1.4f, 1.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.MaxParticles = 11;
		pscd.Emission.SetBurst(1);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 1.f, 1.f, 1.f }, Vec4{ 0.2f, 0.05f, 0.05f, 1.f } }, { 0.f, 1.f });
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Blend;
		pscd.Renderer.TextureName = "Explosion_DoubleFlame";
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Smoke";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 2.f, 3.f };
		pscd.StartSpeed = Vec2{ 0.2f, 0.4f };
		pscd.StartSize = Vec2{ 1.6f, 2.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.35f, 0.35f, 0.35f, 0.35f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.MaxParticles = 20;
		pscd.Emission.SetBurst(5);
		pscd.Emission.SetBurst(5, 0.2f);
		pscd.Shape.SetSphere(0.3f, 1.f, 360.f, false);
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 0.8f }, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 1.f, 1.f, 1.f }, Vec3{ 1.f, 1.f, 1.f }}, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.4f, 0.8f, 0.2f, 0.f }, {0.f, 0.25f, 0.4f, 1.f});
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, 0.2f, 0.f, 0.f } }).SetParam(1.f);
		pscd.RotationOverLifetime.Set(PSValOp::Constant, { 100.f });
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Blend;
		pscd.Renderer.TextureName = "Explosion_Smoke";
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Bullet";
		pscd.Looping = true;
		pscd.Prewarm = false;
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 1.2f, 2.2f };
		pscd.StartSpeed = Vec2{ 0.2f, 0.4f };
		pscd.StartSize = Vec2{ 0.6f, 0.9f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.5f, 0.5f, 0.5f, 0.5f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.f;
		pscd.MaxParticles = 500;
		pscd.Emission.RateOverTime = 100;
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.4f, 1.f }, { 0.f, 1.f });
		pscd.Shape.SetLine();
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.5f, 0.5f, 0.5f, 1.f }, Vec4{ 0.0f, 0.0f, 0.0f, 0.f } }, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.1f, 0.2f, 0.1f, 0.f }, { 0.f, 0.4f, 0.8f, 1.f });
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Blend;
		pscd.Renderer.TextureName = "Explosion_Smoke";
		pscd.Renderer.RenderMode = PSRenderMode::StretchedBillboard;
		pscd.Renderer.LengthScale = 2.5f;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Muzzle_Smoke";
		pscd.StartLifeTime = Vec2{ 2.f };
		pscd.StartSpeed = Vec2{ 0.1f };
		pscd.StartSize = Vec2{ 0.8f, 1.5f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.8f, 0.8f, 0.8f, 0.8f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.2f;
		pscd.MaxParticles = 50;
		pscd.Emission.SetBurst(1);
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.2f, 1.f }, { 0.f, 1.f });
		pscd.Shape.SetSphere(0.2f, 1.f, 360.f, false);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 1.f, 1.f, 1.f }, Vec4{ 0.1f, 0.1f, 0.1f, 0.f } }, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 1.f, 0.f }, { 0.f, 1.f });
		pscd.RotationOverLifetime.Set(PSValOp::Constant, { 70.f });
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, 1.f, 0.f, 0.f } }).SetParam(1.f);
		pscd.Renderer.BlendType = BlendType::Alpha_Blend;
		pscd.Renderer.TextureName = "Explosion_Smoke";
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Muzzle_Flash";
		pscd.StartLifeTime = Vec2{ 0.13f };
		pscd.StartSize = Vec2{ 1.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 2.f, 1.f, 0.5f, 1.f  } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.f;
		pscd.MaxParticles = 50;
		pscd.Emission.SetBurst(1);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 1.f, 1.f, 1.f }, Vec4{ 0.2f, 0.05f, 0.05f, 1.f } }, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({0.4f, 0.f}, {0.f, 1.f});
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Blend;
		pscd.Renderer.TextureName = "Explosion_DoubleFlame";
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.3f, 1.f }, { 0.f, 1.f });
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Nuke_Dot_Sparkles";
		pscd.Duration = 0.5f;
		pscd.StartDelay = 0.05f;
		pscd.StartLifeTime = Vec2{ 0.05f, 1.f };
		pscd.StartSpeed = Vec2{ 2.f, 4.f };
		pscd.StartSize = Vec2{ 1.f, 0.05f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f  } });
		pscd.GravityModifier = 0.01f;
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.6f;
		pscd.MaxAddCount = 1;
		pscd.MaxParticles = 310;
		pscd.Emission.RateOverTime = 300;
		pscd.Shape.SetSphere(0.15f, 1.f, 360.f, true);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 0.9f, 0.47f, 1.f }, Vec4{ 1.f, 0.62f, 0.45f, 1.f }, Vec4{ 1.f, 0.7f, 0.4f, 1.f }, Vec4{ 0.7f, 0.1f, 0.f, 1.f } }, { 0.f, 0.4f, 0.6f, 0.75f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		pscd.Renderer.TextureName = "Explosion_Dot";
		pscd.Renderer.RenderMode = PSRenderMode::StretchedBillboard;
		pscd.Renderer.LengthScale = 2.f;
		pscd.Renderer.BlendType = BlendType::One_To_One_Stretched_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Nuke_Explosion_Add";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 6.f, 8.f };
		pscd.StartSpeed = Vec2{ 0.4f };
		pscd.StartSize = Vec2{ 1.f, 2.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.97f, 0.56f, 0.44f, 1.f  } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.666f;
		pscd.MaxParticles = 40;
		pscd.Emission.SetBurst(10, 0.f);
		pscd.Emission.SetBurst(10, 0.15f);
		pscd.Emission.SetBurst(5, 0.3f);
		pscd.Emission.SetBurst(5, 0.45f);
		pscd.Shape.SetSphere(0.2f, 1.f, 360.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, 1.f, 0.f, 0.f } }).SetParam(0.8f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.f, 0.f, 0.f, 1.f }, Vec4{ 1.f, 0.62f, 0.45f, 1.f }, Vec4{ 0.5f, 0.5f, 0.5f, 1.f }, Vec4{ 0.f, 0.f, 0.f, 1.f } }, { 0.f, 0.2f, 0.5f, 1.f });
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { -60.f, 60.f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.35f, 1.f, 0.f }, { 0.f, 0.2f, 1.f });
		pscd.Renderer.TextureName = "WFX_NukeFlames";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.BlendType = BlendType::One_To_One_Blend_ScrollAlphaMask;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Nuke_Explosion_Smoke";
		pscd.StartDelay = 2.f;
		pscd.StartLifeTime = Vec2{ 3.f, 4.f };
		pscd.StartSpeed = Vec2{ 0.4f };
		pscd.StartSize = Vec2{ 1.f, 2.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.25f, 0.25f, 0.25f, 1.f  } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.5f;
		pscd.MaxParticles = 210;
		pscd.Emission.SetBurst(200, 0.f);
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.f, 2.f }, { 0.f, 1.f });
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ -1.f, 0.f, -1.f, 0.f }, Vec4{ 1.f, 0.f, 1.f, 0.f } }).SetParam(1.f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.6f, 0.6f, 0.6f, 1.f }, Vec4{ 0.f, 0.f, 0.f, 1.f } }, { 0.f, 1.f });
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { -120.f, 120.f });
		pscd.Renderer.TextureName = "WFX_NukeFlames";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.BlendType = BlendType::One_To_One_Blend_ScrollAlphaMask;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Nuke_Smoke_Colon";
		pscd.Duration = 4.f;
		pscd.StartDelay = 1.f;
		pscd.StartLifeTime = Vec2{ 4.f };
		pscd.StartSpeed = Vec2{ 0.f };
		pscd.StartSize = Vec2{ 1.f, 2.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Curve, { Vec4{ 0.33f, 0.33f, 0.33f, 1.f  }, Vec4{ 0.f, 0.f, 0.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.666f;
		pscd.MaxParticles = 70;
		pscd.MaxAddCount = 1;
		pscd.Emission.RateOverTime = 20;
		pscd.Shape.SetSphere(0.8f, 1.f, 360.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, 1.f, 0.f, 0.f } }).SetParam(1.f);
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { 40.f, 70.f });
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.f, 0.f, 0.f, 1.f }, Vec4{ 0.5f, 0.5f, 0.5f, 1.f }, Vec4{ 0.5f, 0.5f, 0.5f, 1.f }, Vec4{ 0.f, 0.f, 0.f, 1.f } }, { 0.f, 0.33f, 0.66f, 1.f });
		pscd.Renderer.TextureName = "WFX_SmokeLoopAlpha";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.BlendType = BlendType::Multiply_Blend_ScrollAlphaMask;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Nuke_Smoke_Head";
		pscd.Duration = 2.f;
		pscd.StartDelay = 4.f;
		pscd.Position = Vec3{ 0.f, 1.3f, 0.f };
		pscd.StartLifeTime = Vec2{ 2.f };
		pscd.StartSpeed = Vec2{ -0.1f };
		pscd.StartSize = Vec2{ 1.f, 2.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.55f, 0.55f, 0.55f, 1.f }});
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.666f;
		pscd.MaxParticles = 50;
		pscd.MaxAddCount = 1;
		pscd.Emission.RateOverTime = 20;
		pscd.Shape.SetSphere(1.3f, 1.f, 360.f, false);
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ -2.f, 0.1f, -2.f, 0.f }, Vec4{ 2.f, 1.f, 2.f, 0.f } }).SetParam(1.f);
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { 30.f, 60.f });
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 0.f, 0.f, 0.f }, Vec3{ 0.62f, 0.62f, 0.62f }, Vec3{ 0.62f, 0.62f, 0.62f }, Vec3{ 0.f, 0.f, 0.f }}, { 0.f, 0.25f, 0.8f, 1.f });
		pscd.Renderer.TextureName = "WFX_SmokeLoopAlpha";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.BlendType = BlendType::Multiply_Blend_ScrollAlphaMask;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Nuke_Explosion_Mult";
		pscd.Duration = 1.5f;
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 6.f, 8.f };
		pscd.StartSpeed = Vec2{ 0.1f };
		pscd.StartSize = Vec2{ 2.2f, 3.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.666f;
		pscd.MaxParticles = 60;
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		pscd.Emission.SetBurst(20);
		pscd.Emission.SetBurst(15, 0.15f);
		pscd.Emission.SetBurst(10, 0.3f);
		pscd.Emission.SetBurst(5, 0.45f);
		pscd.Shape.SetSphere(0.8f, 1.f, 360.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ -0.2f, 1.f, -0.2f, 0.f }, Vec4{ 0.2f, 1.f, 0.2f, 0.f } }).SetParam(1.f);
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { 30.f, 60.f });
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 1.f, 0.85f, 0.61f }, Vec3{ 1.f, 0.6f, 0.4f }, Vec3{ 0.7f, 0.65f, 0.53f }, Vec3{ 0.5f, 0.5f, 0.5f } }, { 0.f, 0.2f, 0.8f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({1.f, 1.f, 1.f, 0.f}, {0.f, 0.5f, 0.8f, 1.f });
		pscd.Renderer.TextureName = "WFX_SmokeLoopAlpha";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.BlendType = BlendType::Scroll_Smoke;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "MagicCircle_Sparks";
		pscd.Looping = true;
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 0.5f };
		pscd.StartSpeed = Vec2{ 0.f, 0.f };
		pscd.StartSize = Vec2{ 0.1f, 0.2f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.8f, 0.3f, 1.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::Local;
		pscd.SimulationSpeed = 0.8f;
		pscd.MaxAddCount = 2;
		pscd.MaxParticles = 300;
		pscd.Emission.RateOverTime = 40;
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 1.f, 0.f }, { 0.f, 1.f });
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Blend;
		pscd.Renderer.TextureName = "Explosion_Grow";
		pscd.Renderer.RenderMode = PSRenderMode::StretchedBillboard;
		pscd.Renderer.LengthScale = 4.f;
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ 0.f, 1.f, 0.f, 0.f }, Vec4{ 0.f, 15.f, 0.f, 0.f } }).SetParam(1.f);
		pscd.Shape.SetSphere(0.9f, 0.01f, 0.f, true);
		ParticleSystem::SavePSCD(pscd);
	}
	{
		ParticleSystemCPUData pscd;
		pscd.mName = "MagicCircle_Dot";
		pscd.Looping = true;
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 1.f };
		pscd.StartSpeed = Vec2{ 0.f, 0.f };
		pscd.StartSize = Vec2{ 1.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.5f, 0.3f, 1.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 0.8f;
		pscd.MaxAddCount = 2;
		pscd.MaxParticles = 100;
		pscd.Emission.RateOverTime = 20;
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.15f, 0.f }, { 0.1f, 1.f });
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Blend;
		pscd.Renderer.TextureName = "Explosion_Grow";
		pscd.Renderer.LengthScale = 5.f;
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ 0.f, 0.6f, 0.f, 0.f }, Vec4{ 0.f, 10.f, 0.f, 0.f } }).SetParam(1.f);
		pscd.Shape.SetSphere(0.9f, 0.01f, 0.f, true);
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "Ursacetus_Smash_Dust";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 7.5f, 11.f };
		pscd.StartSize = Vec2{ 1.f, 6.5f };
		pscd.StartSpeed = Vec2{ 0.5f, 2.5f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.f;
		pscd.MaxParticles = 500;
		pscd.Emission.SetBurst(500, 0.f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.67f, 0.56f, 0.45f, 1.f }, Vec4{ 0.7f, 0.6f, 0.5f, 1.f } }, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.8f, 0.5f, 0.f }, { 0.f, 0.2f, 1.f });
		pscd.Renderer.TextureName = "WFX_SmokeLoopAlpha";
		pscd.Renderer.BlendType = BlendType::Alpha_Stretched_Blend;
		pscd.Shape.SetCone(1.f, 0.1f, 0.1f, true);
		pscd.Position = Vec3{ 0.f, 1.f, 0.f };
		ParticleSystem::SavePSCD(pscd);
	}


	{
		ParticleSystemCPUData pscd;
		pscd.mName = "Ursacetus_Smash_Dust_Ring";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 3.5f, 5.f };
		pscd.StartSize = Vec2{ 3.f, 5.5f };
		pscd.StartSpeed = Vec2{ 0.5f, 8.0f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.f;
		pscd.MaxParticles = 500;
		pscd.Emission.SetBurst(500, 0.1f);
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 1.0f, 3.5f }, { 0.f, 1.f }).SetParam(1.f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.87f, 0.75f, 0.63f, 1.f }, Vec4{ 0.87f, 0.75f, 0.63f, 1.f } }, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.f, 0.3f, 0.f }, { 0.f, 0.2f, 1.f });
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { 10.f, 30.f });
		pscd.Renderer.TextureName = "WFX_SmokeLoopAlpha";
		pscd.Renderer.BlendType = BlendType::Alpha_Blend;
		pscd.Shape.SetCone(0.f, 4.5f, 0.1f, true);
		pscd.Position = Vec3{ 0.f, 1.f, 0.f };
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "Ursacetus_Smash_Dust_Spread";
		pscd.StartDelay = 0.f;
		pscd.StartSpeed = Vec2{ 6.f, 7.f };
		pscd.StartLifeTime = Vec2{ 1.5f, 3.f };
		pscd.StartSize = Vec2{ 0.5f, 0.5f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 2.f;
		pscd.MaxParticles = 200;
		pscd.Emission.SetBurst(150, 0.f);
		pscd.GravityModifier = 2.5f;
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.87f, 0.75f, 0.63f, 1.f }, Vec4{ 0.87f, 0.75f, 0.63f, 1.f } }, { 0.f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.5f, 0.5f }, { 0.f, 1.f });
		pscd.Renderer.TextureName = "WFX_NukeFlames";
		pscd.Renderer.BlendType = BlendType::Alpha_Blend;
		pscd.Shape.SetSphere(1.f, 0.f, 0.f, true);
		pscd.Position = Vec3{ 0.f, 0.f, 0.f };
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Missile_Explosion_Smoke";
		pscd.Duration = 1.5f;
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 10.f, 15.f };
		pscd.StartSpeed = Vec2{ 0.2f, 1.f };
		pscd.Position = Vec3{ 0.f, 0.5f, 0.f };
		pscd.StartSize = Vec2{ 3.f, 3.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.5f;
		pscd.MaxParticles = 20;
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 2.5f, 2.7f, 3.0 }, { 0.f, 0.1f, 1.f });
		pscd.Emission.SetBurst(20);
		pscd.Shape.SetSphere(1.f, 1.0f, 0.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ 0.f, 1.f, 0.f, 0.f }, Vec4{ 0.f, 3.f, 0.f, 0.f } }).SetParam(1.f);
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { 1.f, 1.f });
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 0.35f, 0.35f, 0.35f }, Vec3{ 0.5f, 0.5f, 0.5f }, Vec3{ 0.65f, 0.65f, 0.65f }, Vec3{ 0.8f, 0.8f, 0.8f } }, { 0.f, 0.5f, 0.8f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.8f, 0.5f, 0.3f, 0.f }, { 0.f, 0.2f, 0.4f, 1.f });
		pscd.Renderer.TextureName = "WFX_SmokeLoopAlpha";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.LengthScale = 2.5f;
		pscd.Renderer.BlendType = BlendType::Alpha_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Missile_Dot_Sparkles";
		pscd.Duration = 5.f;
		pscd.GravityModifier = 1.f;
		pscd.StartDelay = 0.05f;
		pscd.StartLifeTime = Vec2{ 2.f, 3.f };
		pscd.StartSpeed = Vec2{ 5.f, 5.f };
		pscd.StartSize = Vec2{ 0.3f, 0.3f};
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f  } });
		pscd.GravityModifier = 1.9f;
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.f;
		pscd.MaxParticles = 200;
		pscd.Emission.SetBurst(200);
		pscd.Shape.SetSphere(0.1f, 0.1f, 360.f, true);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 0.9f, 0.47f, 1.f }, Vec4{ 1.f, 0.62f, 0.45f, 1.f }, Vec4{ 1.f, 0.7f, 0.4f, 1.f }, Vec4{ 0.1f, 0.1f, 0.f, 1.f } }, { 0.f, 0.4f, 0.6f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 1.f, 0.8f, 0.6f, 0.f }, { 0.f, 0.2f, 0.8f, 1.f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		pscd.Renderer.TextureName = "Explosion_Dot";
		pscd.Renderer.RenderMode = PSRenderMode::StretchedBillboard;
		pscd.Renderer.LengthScale = 5.f;
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Stretched_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Missile_Explosion_Add";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 6.f, 8.f };
		pscd.StartSpeed = Vec2{ 0.6f };
		pscd.StartSize = Vec2{ 3.f, 3.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.97f, 0.56f, 0.44f, 1.f  } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 3.666f;
		pscd.MaxParticles = 200;
		pscd.Emission.SetBurst(140, 0.f);
		pscd.Emission.SetBurst(50, 0.15f);
		pscd.Emission.SetBurst(20, 0.3f);
		pscd.Emission.SetBurst(10, 0.45f);
		pscd.Shape.SetSphere(1.f, 1.f, 360.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, 0.1f, 0.f, 0.f } }).SetParam(0.8f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.f, 0.f, 0.f, 1.f }, Vec4{ 1.f, 0.52f, 0.35f, 1.f }, Vec4{ 0.05f, 0.05f, 0.05f, 1.f }, Vec4{ 0.f, 0.f, 0.f, 1.f } }, { 0.f, 0.05f, 0.1f, 1.f });
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { -60.f, 60.f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.35f, 1.f, 0.f }, { 0.f, 0.2f, 1.f });
		pscd.Renderer.TextureName = "WFX_NukeFlames";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Stretched_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Airstrike_Explosion_Smoke";
		pscd.Duration = 1.5f;
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 7.f, 11.f };
		pscd.StartSpeed = Vec2{ 0.2f, 0.5f };
		pscd.Position = Vec3{ 0.f, 1.f, 0.f };
		pscd.StartSize = Vec2{ 1.f, 1.f };
		pscd.StartSize3D = Vec4{ 3.5f, 3.5f, 3.5f, 1.f};
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.9f, 0.4f, 0.3f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.5f;
		pscd.MaxParticles = 50;
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 1.5f, 2.0f, 3.5f }, { 0.f, 0.3f, 1.f });
		pscd.Emission.SetBurst(50);
		pscd.Shape.SetSphere(0.1f, 1.0f, 0.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ 0.5f, 0.5f, 0.f, 0.f }, Vec4{ 1.5f, 2.f, 0.f, 0.f } }).SetParam(1.f);
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { 1.f, 1.f });
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 0.35f, 0.35f, 0.35f }, Vec3{ 0.3f, 0.3f, 0.3f }, Vec3{ 0.25f, 0.25f, 0.25f }, Vec3{ 0.1f, 0.1f, 0.1f } }, { 0.f, 0.5f, 0.8f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.8f, 0.5f, 0.1f, 0.f }, { 0.f, 0.2f, 0.4f, 1.f });
		pscd.Renderer.TextureName = "WFX_SmokeLoopAlpha";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.LengthScale = 1.0f;
		pscd.Renderer.BlendType = BlendType::Alpha_Stretched_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Airstrike_Explosion_Smoke_Spread";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 15.5f, 35.f };
		pscd.StartSpeed = Vec2{ 0.1f, 0.5f };
		pscd.StartSize3D = Vec4{ 3.5f, 3.5f, 3.5f, 1.f};
		pscd.StartSize = Vec2{ 0.3f, 0.5f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 2.5f;
		pscd.GravityModifier = 1.f;
		pscd.MaxParticles = 25;
		pscd.Emission.SetBurst(25, 0.f);
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 2.0f, 2.f, 2.5f, 3.5f }, { 0.f, 0.2f, 0.8f, 1.f });
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ 0.5f, 4.f, 0.f, 0.f }, Vec4{ 1.5f, 7.f, 0.f, 0.f } }).SetParam(1.f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.67f, 0.55f, 0.43f, 1.f }, Vec4{ 0.37f, 0.35f, 0.23f, 1.f }, Vec4{ 0.1f, 0.1f, 0.1f, 1.f } }, { 0.f, 0.6f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.6f, 0.8f, 0.1f, 0.f }, { 0.f, 0.2f, 0.4f, 1.f });
		pscd.Renderer.TextureName = "Explosion_Smoke";
		pscd.Renderer.BlendType = BlendType::Alpha_Blend;
		pscd.Shape.SetSphere(0.5f, 0.f, 360.f, true);
		pscd.Position = Vec3{ 0.f, 1.f, 0.f };
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Airstrike_Dot_Sparkles";
		pscd.Duration = 5.f;
		pscd.GravityModifier = 1.f;
		pscd.StartDelay = 0.05f;
		pscd.StartLifeTime = Vec2{ 2.f, 3.f };
		pscd.StartSpeed = Vec2{ 5.f, 5.f };
		pscd.StartSize = Vec2{ 0.3f, 0.3f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f  } });
		pscd.GravityModifier = 1.9f;
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.f;
		pscd.MaxParticles = 200;
		pscd.Emission.SetBurst(200);
		pscd.Shape.SetSphere(0.1f, 0.1f, 360.f, true);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 0.9f, 0.47f, 1.f }, Vec4{ 1.f, 0.62f, 0.45f, 1.f }, Vec4{ 1.f, 0.7f, 0.4f, 1.f }, Vec4{ 0.1f, 0.1f, 0.f, 1.f } }, { 0.f, 0.4f, 0.6f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 1.f, 0.8f, 0.6f, 0.f }, { 0.f, 0.2f, 0.8f, 1.f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		pscd.Renderer.TextureName = "Explosion_Dot";
		pscd.Renderer.RenderMode = PSRenderMode::StretchedBillboard;
		pscd.Renderer.LengthScale = 5.f;
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Stretched_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_Airstrike_Explosion_Add";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 6.f, 8.f };
		pscd.StartSpeed = Vec2{ 0.6f };
		pscd.StartSize = Vec2{ 1.f, 1.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.97f, 0.56f, 0.44f, 1.f  } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 3.666f;
		pscd.MaxParticles = 50;
		pscd.Emission.SetBurst(50, 0.f);
		pscd.Shape.SetSphere(1.f, 1.f, 360.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, 0.1f, 0.f, 0.f } }).SetParam(0.8f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.f, 0.f, 0.f, 1.f }, Vec4{ 1.f, 0.52f, 0.35f, 1.f }, Vec4{ 0.05f, 0.05f, 0.05f, 1.f }, Vec4{ 0.f, 0.f, 0.f, 1.f } }, { 0.f, 0.05f, 0.1f, 1.f });
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { -60.f, 60.f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.35f, 1.f, 0.f }, { 0.f, 0.2f, 1.f });
		pscd.Renderer.TextureName = "WFX_NukeFlames";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Stretched_Blend;
		ParticleSystem::SavePSCD(pscd);
	}
	
	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_DeusMissile_Explosion_Smoke";
		pscd.Duration = 1.5f;
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 0.2f, 2.f };
		pscd.StartSpeed = Vec2{ 0.2f, 1.f };
		pscd.Position = Vec3{ 0.f, 0.2f, 0.f };
		pscd.StartSize = Vec2{ 1.f, 1.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.5f;
		pscd.MaxParticles = 20;
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 2.5f, 2.7f, 3.0 }, { 0.f, 0.1f, 1.f });
		pscd.Emission.SetBurst(20);
		pscd.Shape.SetSphere(1.f, 1.0f, 0.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ 0.f, 1.f, 0.f, 0.f }, Vec4{ 0.f, 3.f, 0.f, 0.f } }).SetParam(1.f);
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { 1.f, 1.f });
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 0.35f, 0.35f, 0.35f }, Vec3{ 0.5f, 0.5f, 0.5f }, Vec3{ 0.65f, 0.65f, 0.65f }, Vec3{ 0.8f, 0.8f, 0.8f } }, { 0.f, 0.5f, 0.8f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.8f, 0.5f, 0.3f, 0.f }, { 0.f, 0.2f, 0.4f, 1.f });
		pscd.Renderer.TextureName = "WFX_SmokeLoopAlpha";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.LengthScale = 2.5f;
		pscd.Renderer.BlendType = BlendType::Alpha_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_DeusMissile_Dot_Sparkles";
		pscd.Duration = 5.f;
		pscd.GravityModifier = 1.f;
		pscd.StartDelay = 0.05f;
		pscd.StartLifeTime = Vec2{ 2.f, 3.f };
		pscd.StartSpeed = Vec2{ 2.f, 3.f };
		pscd.StartSize = Vec2{ 0.3f, 0.3f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f  } });
		pscd.GravityModifier = 1.9f;
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.f;
		pscd.MaxParticles = 50;
		pscd.Emission.SetBurst(50);
		pscd.Shape.SetSphere(0.1f, 0.1f, 360.f, true);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 1.f, 0.9f, 0.47f, 1.f }, Vec4{ 1.f, 0.62f, 0.45f, 1.f }, Vec4{ 1.f, 0.7f, 0.4f, 1.f }, Vec4{ 0.1f, 0.1f, 0.f, 1.f } }, { 0.f, 0.4f, 0.6f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 1.f, 0.8f, 0.6f, 0.f }, { 0.f, 0.2f, 0.8f, 1.f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.5f, 1.f }, { 0.f, 1.f });
		pscd.Renderer.TextureName = "Explosion_Dot";
		pscd.Renderer.RenderMode = PSRenderMode::StretchedBillboard;
		pscd.Renderer.LengthScale = 5.f;
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Stretched_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_DeusMissile_Explosion_Add";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 6.f, 8.f };
		pscd.StartSpeed = Vec2{ 0.6f };
		pscd.StartSize = Vec2{ 1.f, 1.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.97f, 0.56f, 0.44f, 1.f  } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 3.666f;
		pscd.MaxParticles = 200;
		pscd.Emission.SetBurst(140, 0.f);
		pscd.Emission.SetBurst(50, 0.15f);
		pscd.Emission.SetBurst(20, 0.3f);
		pscd.Emission.SetBurst(10, 0.45f);
		pscd.Shape.SetSphere(1.f, 1.f, 360.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, 0.1f, 0.f, 0.f } }).SetParam(0.8f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.f, 0.f, 0.f, 1.f }, Vec4{ 1.f, 0.52f, 0.35f, 1.f }, Vec4{ 0.05f, 0.05f, 0.05f, 1.f }, Vec4{ 0.f, 0.f, 0.f, 1.f } }, { 0.f, 0.05f, 0.1f, 1.f });
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { -60.f, 60.f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.35f, 1.f, 0.f }, { 0.f, 0.2f, 1.f });
		pscd.Renderer.TextureName = "WFX_NukeFlames";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Stretched_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "WFX_DeusMissile_Explosion_Add2";
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 3.f, 4.f };
		pscd.StartSpeed = Vec2{ 0.3f };
		pscd.StartSize = Vec2{ 1.f, 1.f };
		pscd.StartRotation = Vec2{ 0.f, 360.f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 0.97f, 0.56f, 0.44f, 1.f  } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 3.666f;
		pscd.MaxParticles = 200;
		pscd.Emission.SetBurst(140, 0.f);
		pscd.Emission.SetBurst(50, 0.15f);
		pscd.Emission.SetBurst(20, 0.3f);
		pscd.Emission.SetBurst(10, 0.45f);
		pscd.Shape.SetSphere(1.f, 1.f, 360.f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::Constant, { Vec4{ 0.f, 0.1f, 0.f, 0.f } }).SetParam(0.8f);
		pscd.ColorOverLifetime.Set(PSValOp::Curve, { Vec4{ 0.f, 0.f, 0.f, 1.f }, Vec4{ 1.f, 0.52f, 0.35f, 1.f }, Vec4{ 0.05f, 0.05f, 0.05f, 1.f }, Vec4{ 0.f, 0.f, 0.f, 1.f } }, { 0.f, 0.1f, 0.2f, 1.f });
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { -60.f, 60.f });
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 0.35f, 1.f, 0.f }, { 0.f, 0.2f, 1.f });
		pscd.Renderer.TextureName = "WFX_NukeFlames";
		pscd.Renderer.RenderMode = PSRenderMode::Billboard;
		pscd.Renderer.BlendType = BlendType::Additive_Soft_Stretched_Blend;
		ParticleSystem::SavePSCD(pscd);
	}

	{
		ParticleSystemCPUData pscd;
		pscd.mName = "Scene Dust";
		pscd.Looping = true;
		pscd.StartSpeed = Vec2{ 0.f, 0.f };
		pscd.Position = Vec3{ 0.f, 0.5f, 0.f };
		pscd.StartDelay = 0.f;
		pscd.StartLifeTime = Vec2{ 10.f, 15.f };
		pscd.StartSize = Vec2{ 3.5f, 3.5f };
		pscd.StartColor.Set(PSValOp::Constant, { Vec4{ 1.f, 1.f, 1.f, 1.f } });
		pscd.SimulationSpace = PSSimulationSpace::World;
		pscd.SimulationSpeed = 1.5f;
		pscd.MaxParticles = 1000;
		pscd.SizeOverLifetime.Set(PSValOp::Curve, { 3.5f, 5.f, 3.5f}, { 0.f, 0.5f, 1.f });
		pscd.MaxAddCount = 5;
		pscd.Emission.RateOverTime = 10;
		pscd.Shape.SetCone(0.f, 40.f, 0.1f, true);
		pscd.VelocityOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { Vec4{ 1.f, 0.f, -1.f, 0.f }, Vec4{ 6.f, 2.f, 1.f, 0.f } }).SetParam(1.f);
		pscd.RotationOverLifetime.Set(PSValOp::RandomBetweenTwoConstants, { 1.f, 1.f });
		pscd.ColorOverLifetime.SetColors(PSValOp::Curve, { Vec3{ 0.7f, 0.7f, 0.7f }, Vec3{ 0.8f, 0.8f, 0.8f }, Vec3{ 0.9f, 0.9f, 0.9f }, Vec3{ 0.95f, 0.95f, 0.95f } }, { 0.f, 0.5f, 0.8f, 1.f });
		pscd.ColorOverLifetime.SetAlphas({ 0.f, 0.3f, 0.1f, 0.f }, { 0.f, 0.5f, 0.8f, 1.f });
		pscd.Renderer.TextureName = "WFX_SmokeLoopAlpha";
		pscd.Renderer.RenderMode = PSRenderMode::StretchedBillboard;
		pscd.Renderer.BlendType = BlendType::Alpha_Blend;
		ParticleSystem::SavePSCD(pscd);
	}
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
	std::cout << "Load textures...";

	FileIO::ModelIO::LoadTextures("Import/Textures/Model/");
	FileIO::ModelIO::LoadTextures("Import/Textures/UI/");
	FileIO::ModelIO::LoadTextures("Import/Textures/Terrain/");
	FileIO::ModelIO::LoadTextures("Import/Textures/Skybox/", D3DResource::TextureCube);

	std::cout << "OK\n";
}

void ResourceMgr::LoadModels()
{
	std::cout << "Load models...";

	const std::string rootFolder = "Import/Meshes/";

	sptr<MasterModel> model;
	for (const auto& modelFile : std::filesystem::directory_iterator(rootFolder)) {
		const std::string fileName = modelFile.path().filename().string();
		const std::string modelName = FileIO::RemoveExtension(fileName);

		model = FileIO::ModelIO::LoadGeometryFromFile(rootFolder + fileName);
		ResourceMgr::I->Add(modelName, model);
	}

	std::cout << "OK\n";
}

void ResourceMgr::LoadShaders()
{
	std::cout << "Load shaders...";

// DeferredShader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Global
	{
		ShaderInfo info = {
			ShaderType::Deferred,
		};

		ShaderPath path = {
			 "VShader_Standard.cso",
			 "PShader_Deferred.cso",
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
			 "VShader_Standard.cso",
			 "PShader_Shadow.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Shadow_Global", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::LDR,
		};

		ShaderPath path = {
			 "VShader_Standard.cso",
			 "PShader_DEM_Forward.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("DEM_Global", shader);
	}
#pragma endregion
#pragma region ObjectInst
	{
		ShaderInfo info = {
			ShaderType::Deferred,
			RasterizerType::Cull_None,
		};

		ShaderPath path = {
			 "VShader_StandardInstance.cso",
			 "PShader_Deferred.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("ObjectInst", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::Shadow,
			RasterizerType::DepthBias,
		};

		ShaderPath path = {
			 "VShader_StandardInstance.cso",
			 "PShader_Shadow.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Shadow_ObjectInst", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::LDR,
		};

		ShaderPath path = {
			 "VShader_StandardInstance.cso",
			 "PShader_DEM_Forward.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("DEM_ObjectInst", shader);
	}
#pragma endregion
#pragma region SkinMesh
	{
		ShaderInfo info = {
			ShaderType::Deferred,
		};

		ShaderPath path = {
			 "VShader_SkinnedMesh.cso",
			 "PShader_Deferred.cso",
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
			 "VShader_SkinnedMesh.cso",
			 "PShader_SkinnedMesh_Shadow.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Shadow_SkinMesh", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::Shadow,
			RasterizerType::DepthBias,
			DepthStencilType::Greater,
		};

		ShaderPath path = {
			 "VShader_SkinnedMesh.cso",
			 "PShader_CustomDepth.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("CustomDepth_SkinMesh", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::Less_No_Write,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 "VShader_SkinnedMesh.cso",
			 "PShader_Forward.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Dissolve", shader);
	}	
	{
		ShaderInfo info = {
			ShaderType::LDR,
			RasterizerType::Cull_Back,
			DepthStencilType::Less,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 "VShader_SkinnedMesh.cso",
			 "PShader_DEM_Forward.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("DEM_SkinMesh", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::Less,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 "VShader_SkinnedMesh.cso",
			 "PShader_AfterSkinImage.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("AfterSkinImage", shader);
	}
#pragma endregion
#pragma region Terrain
	{
		ShaderInfo info = {
			ShaderType::Deferred,
		};

		ShaderPath path = {
			 "VShader_Terrain.cso",
			 "PShader_Terrain.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Terrain", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::LDR,
		};

		ShaderPath path = {
			 "VShader_Terrain.cso",
			 "PShader_DEM_Terrain.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("DEM_Terrain", shader);
	}
#pragma endregion

// ForwardShader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Final
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 "VShader_Tex.cso",
			 "PShader_Final.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Final", shader);
	}
#pragma endregion
#pragma region Luminance
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 "VShader_Tex.cso",
			 "PShader_Luminance.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Luminance", shader);
	}
#pragma endregion
#pragma region Bloom
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 "VShader_Tex.cso",
			 "PShader_Bloom.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Bloom", shader);
	}
#pragma endregion
#pragma region DownSampling
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 "VShader_Tex.cso",
			 "PShader_DownSampling.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("DownSampling", shader);
	}
#pragma endregion
#pragma region UpSampling
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 "VShader_Tex.cso",
			 "PShader_UpSampling.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("UpSampling", shader);
	}
#pragma endregion
#pragma region BloomBlur
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_None,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 "VShader_Tex.cso",
			 "PShader_BloomBlur.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("BloomBlur", shader);
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
			 "VShader_Canvas.cso",
			 "PShader_Canvas.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Canvas", shader);
	}
#pragma endregion
#pragma region CooldownCircle
	{
		ShaderInfo info = {
			ShaderType::LDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 "VShader_Canvas.cso",
			 "PShader_CooldownCircle.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("CooldownCircle", shader);
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
			 "VShader_Wired.cso",
			 "PShader_Wired.cso",
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
			 "VShader_Tex.cso",
			 "PShader_OffScreen.cso",
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
			 "VShader_Tex.cso",
			 "PShader_DirLighting.cso",
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
			 "VShader_SpotPointLighting.cso",
			 "PShader_SpotPointLighting.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("SpotPointLighting", shader);
	}
#pragma endregion
#pragma region Transparent
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::Less,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 "VShader_Standard.cso",
			 "PShader_Forward.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Transparent", shader);
	}
#pragma endregion
#pragma region ShieldAbility
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_None,
			DepthStencilType::Less_No_Write,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 "VShader_Standard.cso",
			 "PShader_ShieldAbility.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("ShieldAbility", shader);
	}
#pragma endregion
#pragma region Indicator
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_None,
			DepthStencilType::Less_No_Write,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 "VShader_Standard.cso",
			 "PShader_CircleIndicator.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("CircleIndicator", shader);
	}
#pragma endregion
#pragma region IRDetectorAbility
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 "VShader_Tex.cso",
			 "PShader_IRDetectorAbility.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("IRDetectorAbility", shader);
	}
#pragma endregion
#pragma region NightVisionAbility
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 "VShader_Tex.cso",
			 "PShader_NightVisionAbility.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("NightVisionAbility", shader);
	}
#pragma endregion
#pragma region AerialController
	{
		ShaderInfo info = {
			ShaderType::HDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
		};

		ShaderPath path = {
			 "VShader_Tex.cso",
			 "PShader_AerialController.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("AerialController", shader);
	}
#pragma endregion
#pragma region MinimapAbility
	{
		ShaderInfo info = {
			ShaderType::LDR,
			RasterizerType::Cull_Back,
			DepthStencilType::No_DepthTest_No_Write,
			BlendType::Alpha_Blend,
		};

		ShaderPath path = {
			 "VShader_Canvas.cso",
			 "PShader_MinimapAbility.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("MinimapAbility", shader);
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
			 "VShader_Skybox.cso",
			 "PShader_Skybox.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("SkyBox", shader);
	}
	{
		ShaderInfo info = {
			ShaderType::LDR,
			RasterizerType::Cull_None,
			DepthStencilType::Less_Equal,
		};

		ShaderPath path = {
			 "VShader_Skybox.cso",
			 "PShader_Skybox.cso",
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("DEM_SkyBox", shader);
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
			 "VShader_Ssao.cso",
			 "PShader_Ssao.cso",
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
			 "VShader_Ssao.cso",
			 "PShader_SsaoBlur.cso",
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
			RasterizerType::Cull_None,
			DepthStencilType::Less_No_Write,
			BlendType::Alpha_Blend,
			InputLayoutType::Default,
			D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		};

		ShaderPath path = {
			 "VShader_Particle.cso",
			 "PShader_Particle.cso",
		};

		{
			path.GS = "GShader_Particle.cso";
			info.BlendType = BlendType::Alpha_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("GraphicsParticle", shader);
		}
		{
			path.GS = "GShader_StretchedParticle.cso";
			info.BlendType = BlendType::Alpha_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("GraphicsStretchedParticle", shader);
		}
		{
			path.GS = "GShader_Particle.cso";
			info.BlendType = BlendType::One_To_One_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("OneToOneBlend_GraphicsParticle", shader);
		}
		{
			path.GS = "GShader_StretchedParticle.cso";
			info.BlendType = BlendType::One_To_One_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("OneToOneBlend_GraphicsStretchedParticle", shader);
		}
		{
			path.GS = "GShader_Particle.cso";
			info.BlendType = BlendType::Additive_Soft_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("AdditiveSoft_GraphicsParticle", shader);
		}
		{
			path.GS = "GShader_StretchedParticle.cso";
			info.BlendType = BlendType::Additive_Soft_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("AdditiveSoft_GraphicsStretchedParticle", shader);
		}
		{
			path.GS = "GShader_Particle.cso";
			info.BlendType = BlendType::Multiply_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("MultiplyBlend_GraphicsParticle", shader);
		}
		{
			path.GS = "GShader_StretchedParticle.cso";
			info.BlendType = BlendType::Multiply_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("MultiplyBlend_GraphicsStretchedParticle", shader);
		}

		{
			path.GS = "GShader_Particle.cso";
			path.PS = "PShader_ScrollAlphaMask_Particle.cso";
			info.BlendType = BlendType::One_To_One_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("OneToOneBlend_GraphicsScrollAlphaMaskParticle", shader);
		}

		{
			path.GS = "GShader_Particle.cso";
			path.PS = "PShader_ScrollAlphaMask_Particle.cso";
			info.BlendType = BlendType::Multiply_Inv_Blend;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("MultiplyBlend_GraphicsScrollAlphaMaskParticle", shader);
		}

		{
			path.GS = "GShader_Particle.cso";
			path.PS = "PShader_Scroll_Smoke.cso";
			info.BlendType = BlendType::Scroll_Smoke;
			sptr<Shader> shader = std::make_shared<Shader>();
			shader->Load(info, path);
			Add<Shader>("Scroll_Smoke", shader);
		}
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
			 "",
			 "",
			 "",
			 "CShader_HorzBlur.cso"
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
			 "",
			 "",
			 "",
			 "CShader_VertBlur.cso"
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
			 "",
			 "",
			 "",
			 "CShader_LUT.cso"
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("LUT", shader);
	}
#pragma endregion
#pragma region Sobel
	{
		ShaderInfo info{
			ShaderType::Compute,
		};

		ShaderPath path = {
			 "",
			 "",
			 "",
			 "CShader_Sobel.cso"
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("Sobel", shader);
	}
#pragma endregion
#pragma region ComputeParticle
	{
		ShaderInfo info{
			ShaderType::Particle,
		};

		ShaderPath path = {
			 "",
			 "",
			 "",
			 "CShader_Particle.cso"
		};

		sptr<Shader> shader = std::make_shared<Shader>();
		shader->Load(info, path);
		Add<Shader>("ComputeParticle", shader);
	}
#pragma endregion

	std::cout << "OK\n";
}

void ResourceMgr::LoadAnimationClips()
{
	std::cout << "Load animation clips...";

	const std::string rootFolder = "Import/AnimationClips/";
	for (const auto& clipFolder : std::filesystem::directory_iterator(rootFolder)) {
		std::string clipFolderName = clipFolder.path().filename().string();

		for (const auto& file : std::filesystem::directory_iterator(rootFolder + clipFolderName + '/')) {
			std::string fileName = file.path().filename().string();
			sptr<AnimationClip> clip = FileIO::AnimationIO::LoadAnimationClip(clipFolder.path().string() + '/' + fileName);

			FileIO::RemoveExtension(fileName);
			const std::string clipName = clipFolderName + '/' + fileName;
			ResourceMgr::I->Add<AnimationClip>(clipName, clip);
		}
	}

	std::cout << "OK\n";
}

void ResourceMgr::LoadAnimatorControllers()
{
	const std::string rootFolder = "Import/AnimatorControllers/";
	for (const auto& file : std::filesystem::directory_iterator(rootFolder)) {
		const std::string fileName = file.path().filename().string();
		ResourceMgr::I->Add<AnimatorController>(FileIO::RemoveExtension(fileName), FileIO::AnimationIO::LoadAnimatorController(rootFolder + fileName));
	}
}

void ResourceMgr::LoadParticleSystemCPUData()
{
	const std::string rootFolder = "Import/ParticleSystems/";

	ResourceMgr::I->RemoveAll<ParticleSystemCPUData>();
	ResourceMgr::I->RemoveAll<ParticleSystemGPULoadData>();
	for (const auto& file : std::filesystem::directory_iterator(rootFolder)) {
		const std::string fileName = file.path().filename().string();
		const std::string resName = FileIO::RemoveExtension(fileName);
		ResourceMgr::I->Add<ParticleSystemCPUData>(resName, ParticleSystem::LoadPSCD(rootFolder + fileName));
		ResourceMgr::I->Add<ParticleSystemGPULoadData>(resName, ParticleSystem::LoadPSGD(resName));
	}
}

void ResourceMgr::LoadSounds()
{
	std::cout << "Load Sounds...";

	SoundMgr::I->LoadSounds();

	std::cout << "OK\n";
}
