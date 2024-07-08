#include "stdafx.h"
#include "Script_Item.h"

#include "ScriptExporter.h"

void Script_Item_WeaponCrate::Awake()
{
	
	mObject->RemoveComponent<ScriptExporter>();
}

void Script_Item_WeaponCrate::LoadData(rsptr<ScriptExporter> exporter)
{
	exporter->GetData("Type", type);
	exporter->GetData("Count", count);
	exporter->GetData("hp", hp);
	exporter->GetData("can_open", can_open);
}

void Script_Item_WeaponCrate::Interact()
{
	std::cout << "Interact Crate : " << "Type : " << type << ", Count : " << count << ", Hp : " << hp << ", can_open : " << can_open << std::endl;
}