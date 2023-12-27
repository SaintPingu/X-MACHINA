#pragma once

struct LIGHT;
struct MATERIALLOADINFO;

class CModel;
class CMasterModel;
class CMeshLoadInfo;
class CMaterial;

class FileMgr {
public:
	// Model
	//static std::vector<MATERIALLOADINFO> LoadMaterialsInfoFromFile(FILE* file);
	static std::vector<sptr<CMaterial>> LoadMaterialsFromFile(FILE* file);
	static sptr<CMeshLoadInfo> LoadMeshInfoFromFile(FILE* file);

	static sptr<CModel> LoadFrameHierarchyFromFile(FILE* file);
	static sptr<CMasterModel> LoadGeometryFromFile(const std::string& fileName);

	// Light
	static void LoadLightFromFile(const std::string& fileName, LIGHT** out);
};
