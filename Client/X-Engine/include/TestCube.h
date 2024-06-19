#pragma once

#pragma region Include
#include "Component/Transform.h"
#include "Component/Component.h"
#pragma endregion

#pragma region ClassForwardDecl
class Texture;
class Camera;
class ModelObjectMesh;
class Material;
#pragma endregion

// �׽�Ʈ �뵵 ť��
class TestCube : public Object {
private:
	float mSize = 2.f;

	uptr<ModelObjectMesh>	mMesh{};
	sptr<Material>			mMaterial{};
	
public:
	TestCube(Vec2 pos);
	virtual ~TestCube() = default;

public:
	rsptr<Material> GetMaterial() { return mMaterial; }

	void Render();
};

