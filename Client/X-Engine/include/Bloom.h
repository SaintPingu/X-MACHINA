#pragma once


#pragma region ClassForwardDecl
class Texture;
#pragma endregion


#pragma region Class
class Bloom {
public:
	static constexpr int mkSamplingCount = 3;

private:
	sptr<Texture> mLuminance;
	std::array<sptr<Texture>, mkSamplingCount> mDownSamples;
	std::array<sptr<Texture>, mkSamplingCount> mUpSamples;
	
public:
#pragma region C/Dtor
	Bloom();
	virtual ~Bloom() = default;
#pragma endregion

public:
	void Execute();

private:
	void DownSampling();
	void UpSampling();
};
#pragma endregion
