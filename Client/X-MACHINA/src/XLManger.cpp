#include "stdafx.h"
#include "XLManager.h"

#include "Script_Phero.h"
#include "Script_PheroObject.h"


void XLManger::LoadTables()
{
	mTables[static_cast<UINT8>(XLDataType::PheroStat)] = std::make_shared<PheroStatTable>();
	mTables[static_cast<UINT8>(XLDataType::PheroStat)]->Load("Import/XL/Phero/PheroStat.xlsx");

	mTables[static_cast<UINT8>(XLDataType::PheroObjectStat)] = std::make_shared<PheroObjectStatTable>();
	mTables[static_cast<UINT8>(XLDataType::PheroObjectStat)]->Load("Import/XL/Phero/PheroObjectStat.xlsx");
}
