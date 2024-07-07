#include "EnginePch.h"

#include "ScriptExporter.h"
#include "FileIO.h"

void ScriptExporter::Load(std::ifstream& file)
{
	std::string token{};

	FileIO::ReadString(file, token);	// <ScriptName>:
	FileIO::ReadString(file, mName);

	FileIO::ReadString(file, token);	// <ScriptInfo>:
	int cnt = FileIO::ReadVal<int>(file);

	for (int i = 0; i < cnt; ++i) {
		std::string key = FileIO::ReadString(file);
		char type = key.back();
		key.pop_back();

		ScriptParameter param;
		switch (type) {
		case 'i':
			param.Type = ScriptParameter::Type::Int;
			FileIO::ReadVal(file, param.Val.i);
			break;
		case 'f':
			param.Type = ScriptParameter::Type::Float;
			FileIO::ReadVal(file, param.Val.f);
			break;
		case 'b':
			param.Type = ScriptParameter::Type::Bool;
			FileIO::ReadVal(file, param.Val.b);
			break;
		case 's':
			param.Type = ScriptParameter::Type::String;
			FileIO::ReadString(file, param.Str);
			break;
		default:
			assert(0);
			break;
		}

		mData[key] = param;
	}
}
