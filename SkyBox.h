#pragma once
#include "Transform.h"

class CTexture;
class Camera;
class CSkyBoxShader;
class CSkyBoxMesh;

class CSkyBox : public Transform {
private:
	std::unique_ptr<CSkyBoxShader> mShader{};
	std::unique_ptr<CTexture> mTexture{};
	std::unique_ptr<CSkyBoxMesh> mMesh{};

public:
	CSkyBox();
	~CSkyBox() = default;

	void Render();
};

