#pragma once


#pragma region ClassForwardDecl
class Texture;
#pragma endregion


#pragma region Class
class Bloom {
private:
	std::array<sptr<Texture>, BloomCount> mTargets;
	std::stack<sptr<Texture>> mBaseTargets;
	sptr<Texture> mOutput;

public:
#pragma region C/Dtor
	Bloom();
	virtual ~Bloom() = default;
#pragma endregion

public:
	void Execute();
	UINT GetOutput();

private:
	void ExtractLuminace();

	void DownSampling();
	void UpSampling();

	void Blur(int vertBlur, const Vec2& renderTargetSize);
};
#pragma endregion
