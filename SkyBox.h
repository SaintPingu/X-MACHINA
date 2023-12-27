#pragma once
#include "Transform.h"

class Texture;
class Camera;
class SkyBoxShader;
class SkyBoxMesh;

class SkyBox : public Transform {
private:
	std::unique_ptr<SkyBoxShader> mShader{};
	std::unique_ptr<Texture> mTexture{};
	std::unique_ptr<SkyBoxMesh> mMesh{};

public:
	SkyBox();
	~SkyBox() = default;

	void Render();
};

