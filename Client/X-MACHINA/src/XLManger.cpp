#include "stdafx.h"
#include "XLManager.h"

#include "Script_Phero.h"


void XLManger::LoadTables()
{
	mTables[static_cast<UINT8>(XLDataType::PheroStat)] = std::make_shared<PheroStatTable>();
	mTables[static_cast<UINT8>(XLDataType::PheroStat)]->Load("Import/XL/PheroStat/PheroStat.xlsx");
}
