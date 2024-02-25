#pragma once

#pragma region ClassForwardDecl
struct LightLoadInfo;
struct MeshLoadInfo;
struct AnimationLoadInfo;

class Model;
class MasterModel;
class Material;
class Texture;
class AnimationClip;
class AnimatorController;
#pragma endregion

namespace FileIO {
	// 단일 문자열을 읽어 out으로 반환한다.
	void ReadString(FILE* file, std::string& out);

	// T의 size만큼 file의 내용을 읽어 out으로 반환한다.
	template<class T>
	inline void ReadVal(FILE* file, T& out)
	{
		::fread(&out, sizeof(T), 1, file);
	}

	// T의 size만큼 file의 내용을 읽어 반환한다.
	template<class T>
	inline T ReadVal(FILE* file)
	{
		T val;
		::fread(&val, sizeof(T), 1, file);
		return val;
	}

	// T의 size * (cnt)만큼 file의 내용을 읽어 out으로 반환한다.
	template<class T>
	inline void ReadRange(FILE* file, std::vector<T>& out, int cnt)
	{
		out.resize(cnt);
		::fread(out.data(), sizeof(T), cnt, file);
	}

	// remove extension of [filePath]
	inline void RemoveExtension(std::string& fileName)
	{
		fileName = fileName.substr(0, fileName.find_last_of('.'));
	}

	inline std::string RemoveExtension(const std::string& fileName)
	{
		return fileName.substr(0, fileName.find_last_of('.'));
	}

	// filePath에 해당하는 모델을 불러온다. (계층구조)
	sptr<MasterModel> LoadGeometryFromFile(const std::string& filePath);

	void LoadAnimation(FILE* file, sptr<AnimationLoadInfo>& animationInfo);
	sptr<AnimationClip> LoadAnimationClip(const std::string& filePath);

	sptr<AnimatorController> LoadAnimatorController(const std::string& filePath);

	// [filePath]에 해당하는 조명 모델을 불러온다. (Type, Color, Intensity, ...)
	void LoadLightFromFile(const std::string& filePath, LightLoadInfo** out);

	// [folder]의 모든 dds Texutre파일들을 로드한다.
	// <texture name, Texture>
	void LoadTextures(const std::string& folder);
}