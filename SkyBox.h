#pragma once

#pragma region Include
#include "Transform.h"
#pragma endregion


#pragma region ClassForwardDecl
class Texture;
class Camera;
class SkyBoxShader;
class SkyBoxMesh;
#pragma endregion


#pragma region Class
class SkyBox : public Transform {
private:
	std::unique_ptr<SkyBoxShader> mShader{};
	std::unique_ptr<Texture> mTexture{};
	std::unique_ptr<SkyBoxMesh> mMesh{};

public:
	SkyBox();
	virtual ~SkyBox() = default;

public:
	void Render();
};
#pragma endregion