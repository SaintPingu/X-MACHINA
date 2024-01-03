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

	// fileName에 해당하는 모델을 불러온다. (계층구조)
	sptr<MasterModel> LoadGeometryFromFile(const std::string& fileName);

	// [fileName]에 해당하는 조명 모델을 불러온다. (Type, Color, Intensity, ...)
	void LoadLightFromFile(const std::string& fileName, LightInfo** out);

	// [folder]의 모든 dds Texutre파일들을 로드한다.
	// <texture name, Texture>
	std::unordered_map<std::string, sptr<Texture>> LoadTextures(const std::string& folder);

	// remove extension of [fileName]
	inline void RemoveExtension(std::string& filename)
	{
		filename = filename.substr(0, filename.find_last_of('.'));
	}
}