#pragma once

#pragma region ClassForwardDecl
struct LightInfo;
struct MeshLoadInfo;

class Model;
class MasterModel;
class Material;
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

	/* Model */
	std::vector<sptr<Material>> LoadMaterial(FILE* file);
	sptr<MeshLoadInfo> LoadMesh(FILE* file);

	sptr<Model> LoadFrrameHierarchy(FILE* file);
	sptr<MasterModel> LoadGeometryFromFile(const std::string& fileName);

	/* Light */
	void LoadLightFromFile(const std::string& fileName, LightInfo** out);

	/* Texture */
	void GetTextureNames(std::vector<std::string>& out, const std::string& folder);
}