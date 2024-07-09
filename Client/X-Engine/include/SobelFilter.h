#pragma once


#pragma region ClassForwardDecl
class Texture;
#pragma endregion


class SobelFilter : private UnCopyable {
private:
	// output resource
	sptr<Texture> mOutput{};

	UINT mWidth{};
	UINT mHeight{};

public:
#pragma region C/Dtor
	SobelFilter(UINT width, UINT height);
	virtual ~SobelFilter() = default;
#pragma endregion

public:
	void Create();
	void OnResize(UINT width, UINT height);
	UINT Execute(rsptr<Texture> input);

private:
	void CreateResource();
};

