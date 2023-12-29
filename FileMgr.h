#pragma once

//-----------------------------[Class Declaration]-----------------------------//
struct LIGHT;
struct MATERIALLOADINFO;

class Model;
class MasterModel;
class MeshLoadInfo;
class Material;
//-----------------------------------------------------------------------------//

namespace IO {
	int ReadUnityBinaryString(FILE* file, std::string& token);
	int ReadIntegerFromFile(FILE* file);
	float ReadFloatFromFile(FILE* file);
	UINT ReadStringFromFile(FILE* file, std::string& token);
}

namespace FileMgr {
	// Model
	std::vector<sptr<Material>> LoadMaterialsFromFile(FILE* file);
	sptr<MeshLoadInfo> LoadMeshInfoFromFile(FILE* file);

	sptr<Model> LoadFrameHierarchyFromFile(FILE* file);
	sptr<MasterModel> LoadGeometryFromFile(const std::string& fileName);

	// Light
	void LoadLightFromFile(const std::string& fileName, LIGHT** out);

	// texture
	void GetTextureNames(std::vector<std::string>& out, const std::string& folder);
};
