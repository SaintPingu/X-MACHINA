#pragma once


#pragma region ClassForwardDecl
struct PheroStat;
struct PheroObjectStat;
#pragma endregion


#pragma region EnumClass
enum class XLDataType : UINT8 {
	PheroStat = 0,
	PheroObjectStat,

	_count
};

enum {
	XLDataTypeCount = static_cast<UINT8>(XLDataType::_count)
};
#pragma endregion


#pragma region Struct
struct XLData {
	virtual ~XLData() = default;
};

struct XLTable {
	std::vector<sptr<XLData>> mDatas{};

	virtual void Load(const std::string& path) abstract;
	const sptr<XLData>& Get(int row) { return mDatas[row]; }
};
#pragma endregion


#pragma region Class
class XLManger : public Singleton<XLManger> {
	friend Singleton;

private:
	std::array<sptr<XLTable>, XLDataTypeCount> mTables;

public:
#pragma region C/Dtor
	XLManger() = default;
	virtual ~XLManger() = default;
#pragma endregion

public:
	void LoadTables();

	template<typename T>
	sptr<T> Get(int row);

	template<typename T>
	void Set(int row, T& data);

	template<typename T>
	XLDataType GetXLDataType();
};
#pragma endregion

template<typename T>
inline sptr<T> XLManger::Get(int row)
{
	XLDataType dataType = GetXLDataType<T>();
	auto& table = mTables[static_cast<UINT8>(dataType)];

	if (table->mDatas.size() < row) {
		return nullptr;
	}

	return std::dynamic_pointer_cast<T>(table->Get(row));
}

template<typename T>
inline void XLManger::Set(int row, T& data)
{
	auto& tableData = XLManger::I->Get<T>(row);
	if (nullptr != tableData) {
		data = *tableData.get();
	}
}

template<typename T>
inline XLDataType XLManger::GetXLDataType()
{
	if (std::is_same_v<T, PheroStat>)
		return XLDataType::PheroStat;
	else if (std::is_same_v<T, PheroObjectStat>)
		return XLDataType::PheroObjectStat;
}
