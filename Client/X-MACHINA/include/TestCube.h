#pragma once

#pragma region Include
#include "Transform.h"
#pragma endregion

#pragma region ClassForwardDecl
class Texture;
class Camera;
class DeferredShader;
class ModelObjectMesh;
class Material;
#pragma endregion

// 테스트 용도 큐브
class TestCube : public Transform {
private:
	float size = 20.f;

	uptr<ModelObjectMesh>	mMesh{};
	uptr<DeferredShader>	mShader{};
	sptr<Material>			mMaterial{};
	
public:
	TestCube(Vec2 pos);
	virtual ~TestCube() = default;

public:
	rsptr<Material> GetMaterial() { return mMaterial; }

	void Render();
};

