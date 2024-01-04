#pragma once

#pragma region Include
#include "Transform.h"
#pragma endregion


#pragma region ClassForwardDecl
class Texture;
class Camera;
class SkyBoxShader;
class ModelObjectMesh;
#pragma endregion


#pragma region Class
// SkyBox ��� ��ü (not entity)
class SkyBox : public Transform {
private:
	uptr<ModelObjectMesh> mMesh{};
	uptr<SkyBoxShader>	 mShader{};
	uptr<Texture>		 mTexture{};

public:
	SkyBox();
	virtual ~SkyBox() = default;

public:
	void Render();
};
#pragma endregion