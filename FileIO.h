#pragma once

#pragma region ClassForwardDecl
struct LightInfo;
struct MeshLoadInfo;

class Model;
class MasterModel;
class Material;
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