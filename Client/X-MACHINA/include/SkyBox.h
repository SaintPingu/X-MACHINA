#pragma once

#pragma region Include
#include "Transform.h"
#pragma endregion


#pragma region ClassForwardDecl
class Texture;
class Camera;
class ModelObjectMesh;
#pragma endregion


#pragma region Class
// SkyBox ¹è°æ °´Ã¼ (not entity)
class SkyBox : public Transform {
private:
	uptr<ModelObjectMesh> mMesh{};
	sptr<Texture>		 mTexture{};

public:
	SkyBox();
	virtual ~SkyBox() = default;

public:
	void SetGraphicsRootDescriptorTable();
	void Render();
};
#pragma endregion