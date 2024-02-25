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
	// ���� ���ڿ��� �о� out���� ��ȯ�Ѵ�.
	void ReadString(FILE* file, std::string& out);

	// T�� size��ŭ file�� ������ �о� out���� ��ȯ�Ѵ�.
	template<class T>
	inline void ReadVal(FILE* file, T& out)
	{
		::fread(&out, sizeof(T), 1, file);
	}

	// T�� size��ŭ file�� ������ �о� ��ȯ�Ѵ�.
	template<class T>
	inline T ReadVal(FILE* file)
	{
		T val;
		::fread(&val, sizeof(T), 1, file);
		return val;
	}

	// T�� size * (cnt)��ŭ file�� ������ �о� out���� ��ȯ�Ѵ�.
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

	// filePath�� �ش��ϴ� ���� �ҷ��´�. (��������)
	sptr<MasterModel> LoadGeometryFromFile(const std::string& filePath);

	void LoadAnimation(FILE* file, sptr<AnimationLoadInfo>& animationInfo);
	sptr<AnimationClip> LoadAnimationClip(const std::string& filePath);

	sptr<AnimatorController> LoadAnimatorController(const std::string& filePath);

	// [filePath]�� �ش��ϴ� ���� ���� �ҷ��´�. (Type, Color, Intensity, ...)
	void LoadLightFromFile(const std::string& filePath, LightLoadInfo** out);

	// [folder]�� ��� dds Texutre���ϵ��� �ε��Ѵ�.
	// <texture name, Texture>
	void LoadTextures(const std::string& folder);
}