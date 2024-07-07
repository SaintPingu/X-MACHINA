#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

#pragma region Struct
struct ScriptParameter {
	enum class Type : BYTE {
		None,
		String,
		Float,
		Int,
		Bool,
	};

	Type Type{};
	std::string Str{};
	union value {
		bool b;
		int i;
		float f;
	} Val{};
};
#pragma endregion


#pragma region Class
class ScriptExporter : public Component {
	COMPONENT(ScriptExporter, Component)

private:
	std::string mName{};
	std::unordered_map<std::string, ScriptParameter> mData{};

public:
	const std::string& GetName() { return mName; }
	template<class T>
	void GetData(const std::string& key, T& out)
	{
		if (!mData.contains(key)) {
			std::cout << "[ERROR - ScriptExporter] data name mismatch.\n";
			assert(0);
			return;
		}
		auto& data = mData[key];

		if constexpr (std::is_same_v<T, std::string>) {
			out = data.Str;
		}
		else if constexpr (std::is_same_v<T, int>) {
			out = data.Val.i;
		}
		else if constexpr (std::is_same_v<T, float>) {
			out = data.Val.f;
		}
		else if constexpr (std::is_same_v<T, bool>) {
			out = data.Val.b;
		}
	}

public:
	void Load(std::ifstream& file);
};
#pragma endregion
