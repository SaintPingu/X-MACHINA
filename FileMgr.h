#pragma once

struct LIGHT;
struct MATERIALLOADINFO;

class Model;
class MasterModel;
class MeshLoadInfo;
class Material;

class FileMgr {
public:
	// Model
	//static std::vector<MATERIALLOADINFO> LoadMaterialsInfoFromFile(FILE* file);
	static std::vector<sptr<Material>> LoadMaterialsFromFile(FILE* file);
	static sptr<MeshLoadInfo> LoadMeshInfoFromFile(FILE* file);

	static sptr<Model> LoadFrameHierarchyFromFile(FILE* file);
	static sptr<MasterModel> LoadGeometryFromFile(const std::string& fileName);

	// Light
	static void LoadLightFromFile(const std::string& fileName, LIGHT** out);
};
