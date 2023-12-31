#pragma once

#pragma region ClassForwardDecl
struct LightInfo;
struct MeshLoadInfo;

class Model;
class MasterModel;
class Material;
#pragma endregion

namespace FileIO {
	/* Read Single */
	void ReadString(FILE* file, std::string& out);

	template<class T>
	inline void ReadVal(FILE* file, T& out)
	{
		::fread(&out, sizeof(T), 1, file);
	}

	template<class T>
	inline T ReadVal(FILE* file)
	{
		T val;
		::fread(&val, sizeof(T), 1, file);
		return val;
	}

	template<class T>
	inline void ReadRange(FILE* file, std::vector<T>& out, int size)
	{
		::fread(out.data(), sizeof(T), size, file);
	}

	/* Model */
	std::vector<sptr<Material>> LoadMaterialsFromFile(FILE* file);
	sptr<MeshLoadInfo> LoadMeshInfoFromFile(FILE* file);

	sptr<Model> LoadFrameHierarchyFromFile(FILE* file);
	sptr<MasterModel> LoadGeometryFromFile(const std::string& fileName);

	/* Light */
	void LoadLightFromFile(const std::string& fileName, LightInfo** out);

	/* Texture */
	void GetTextureNames(std::vector<std::string>& out, const std::string& folder);
}