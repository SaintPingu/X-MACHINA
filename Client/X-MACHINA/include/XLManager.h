#pragma once


#pragma region Include
#include <variant>
#pragma endregion


#pragma region ClassForwardDecl
struct PheroStat;
struct PheroObjectStat;
struct EnemyStat;
#pragma endregion


#pragma region EnumClass
enum class XLDataType : UINT8 {
	PheroStat = 0,
	PheroObjectStat,
	EnemyStat,

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

// 행(숫자)를 통해 찾을 수 있는 테이블
struct XLTable {
	using ContainerVariant = std::variant<
		std::vector<sptr<XLData>>, // 행 숫자로 찾는 경우
		std::unordered_map<std::string, sptr<XLData>> // 행 문자열로 찾는 경우
	>;

	ContainerVariant mDatas{};
	std::vector<std::string> mKeys{};

	XLTable(bool useStringKey) {
		if (!useStringKey) {
			mDatas = std::vector<sptr<XLData>>(1);
		}
		else {
			mDatas = std::unordered_map<std::string, sptr<XLData>>();
		}
	}

	virtual void Load(const std::string& path) {
		xlnt::workbook wb;
		wb.load(path);
		auto ws = wb.active_sheet();

		// 행 숫자로 값을 찾는 경우 1열부터 value 값이어야 한다.
		if (std::holds_alternative<std::vector<std::shared_ptr<XLData>>>(mDatas)) {
			auto datas = std::get_if<std::vector<std::shared_ptr<XLData>>>(&mDatas);

			bool firstRowSkipped = false;
			for (const auto& row : ws.rows(false)) {
				if (!firstRowSkipped) {
					firstRowSkipped = true;
					continue;
				}

				datas->push_back(SetData(row));
			}
		}
		// 문자열로 값을 찾는 경우 1열은 key 값이어야 한다.
		else {
			auto datas = std::get_if<std::unordered_map<std::string, sptr<XLData>>>(&mDatas);

			int cnt{};
			bool firstRowSkipped = false;
			for (auto& row : ws.rows(false)) {
				if (!firstRowSkipped) {
					firstRowSkipped = true;
					continue;
				}

				(*datas)[mKeys[cnt++]] = SetData(row);
			}
		}
	}

	virtual sptr<XLData> SetData(const xlnt::range_iterator::reference& row) abstract;

	const sptr<XLData> Get(int row) const {
		auto vec = std::get_if<std::vector<sptr<XLData>>>(&mDatas);

		if (row < 0 || row >= static_cast<int>(vec->size())) {
			return nullptr;
		}

		return (*vec)[row];
	}

	const sptr<XLData> Get(const std::string& key) const {
		auto map = std::get_if<std::unordered_map<std::string, sptr<XLData>>>(&mDatas);
		auto findIt = map->find(key);
		
		if (findIt == map->end()) {
			return nullptr;
		}

		return findIt->second;
	}
};
#pragma endregion


#pragma region Class
class XLManager : public Singleton<XLManager> {
	friend Singleton;

private:
	std::array<sptr<XLTable>, XLDataTypeCount> mTables;

public:
#pragma region C/Dtor
	XLManager() = default;
	virtual ~XLManager() = default;
#pragma endregion

public:
	void LoadTables();

	template<typename T>
	sptr<T> Get(int row);

	template<typename T>
	sptr<T> Get(std::string key);

	template<typename T>
	void Set(int row, T& data);

	template<typename T>
	void Set(std::string key, T& data);

	template<typename T>
	XLDataType GetXLDataType();
};
#pragma endregion

template<typename T>
inline sptr<T> XLManager::Get(int row)
{
	XLDataType dataType = GetXLDataType<T>();
	auto& table = mTables[static_cast<UINT8>(dataType)];

	return std::dynamic_pointer_cast<T>(table->Get(row));
}

template<typename T>
inline sptr<T> XLManager::Get(std::string key)
{
	XLDataType dataType = GetXLDataType<T>();
	auto& table = mTables[static_cast<UINT8>(dataType)];

	return std::dynamic_pointer_cast<T>(table->Get(key));
}

template<typename T>
inline void XLManager::Set(int row, T& data)
{
	auto& tableData = XLManager::I->Get<T>(row);
	if (nullptr != tableData) {
		data = *tableData.get();
	}
}

template<typename T>
inline void XLManager::Set(std::string key, T& data)
{
	auto& tableData = XLManager::I->Get<T>(key);
	if (nullptr != tableData) {
		data = *tableData.get();
	}
}

template<typename T>
inline XLDataType XLManager::GetXLDataType()
{
	if (std::is_same_v<T, PheroStat>)
		return XLDataType::PheroStat;
	else if (std::is_same_v<T, PheroObjectStat>)
		return XLDataType::PheroObjectStat;
	else if (std::is_same_v<T, EnemyStat>)
		return XLDataType::EnemyStat;
}
