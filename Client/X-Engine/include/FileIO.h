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
	inline void ReadString(std::ifstream& file, std::string& out)
	{
		BYTE length{};
		file.read(reinterpret_cast<char*>(&length), sizeof(BYTE));
		out.resize(length);
		file.read(reinterpret_cast<char*>(out.data()), length);
	}
	inline std::string ReadString(std::ifstream& file)
	{
		std::string out;
		FileIO::ReadString(file, out);
		return out;
	}

	// T�� size��ŭ file�� ������ �о� out���� ��ȯ�Ѵ�.
	template<typename T, typename = std::enable_if_t<!std::is_same<T, std::string>::value>>
	inline void ReadVal(std::ifstream& file, T& out)
	{
		file.read(reinterpret_cast<char*>(&out), sizeof(T));
	}

	// T�� size��ŭ file�� ������ �о� ��ȯ�Ѵ�.
	template<typename T, typename = std::enable_if_t<!std::is_same<T, std::string>::value>>
	inline T ReadVal(std::ifstream& file)
	{
		T val;
		file.read(reinterpret_cast<char*>(&val), sizeof(T));
		return val;
	}

	// T�� size * (cnt)��ŭ file�� ������ �о� out���� ��ȯ�Ѵ�.
	template<class T>
	inline void ReadRange(std::ifstream& file, std::vector<T>& out, int cnt)
	{
		out.resize(cnt);
		file.read(reinterpret_cast<char*>(out.data()), sizeof(T) * cnt);
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

	namespace ModelIO {
		// filePath�� �ش��ϴ� ���� �ҷ��´�. (��������)
		sptr<MasterModel> LoadGeometryFromFile(const std::string& filePath);

		// [filePath]�� �ش��ϴ� ���� ���� �ҷ��´�. (Type, Color, Intensity, ...)
		void LoadLightFromFile(const std::string& filePath, LightLoadInfo** out);

		// [folder]�� ��� dds Texutre���ϵ��� �ε��Ѵ�.
		// <texture name, Texture>
		void LoadTextures(const std::string& folder, D3DResource textureType = D3DResource::Texture2D);
	}

	namespace AnimationIO {
		void SetAnimation(std::ifstream& file, sptr<AnimationLoadInfo>& animationInfo);
		sptr<AnimationClip> LoadAnimationClip(const std::string& filePath);

		sptr<AnimatorController> LoadAnimatorController(const std::string& filePath);
	}

	inline std::ifstream OpenBinFile(const std::string& filePath)
	{
		std::ifstream file(filePath, std::ios::binary);
		if (!file) {
			throw std::runtime_error("Can not open the file : " + filePath);
		}

		return file;
	}
}