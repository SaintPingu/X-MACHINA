#pragma once

#pragma region ClassForwardDecl
struct LightInfo;
struct MeshLoadInfo;

class Model;
class MasterModel;
class Material;
class Texture;
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

	// fileName�� �ش��ϴ� ���� �ҷ��´�. (��������)
	sptr<MasterModel> LoadGeometryFromFile(const std::string& fileName);

	// [fileName]�� �ش��ϴ� ���� ���� �ҷ��´�. (Type, Color, Intensity, ...)
	void LoadLightFromFile(const std::string& fileName, LightInfo** out);

	// [folder]�� ��� dds Texutre���ϵ��� �ε��Ѵ�.
	// <texture name, Texture>
	std::unordered_map<std::string, sptr<Texture>> LoadTextures(const std::string& folder);

	// remove extension of [fileName]
	inline void RemoveExtension(std::string& filename)
	{
		filename = filename.substr(0, filename.find_last_of('.'));
	}
}