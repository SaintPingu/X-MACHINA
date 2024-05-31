#include "stdafx.h"
#include "Script_PheroObject.h"

#include "Scene.h"
#include "ObjectPool.h"
#include "Object.h"

void Script_PheroObject::Start()
{
	base::Start();


}

void Script_PheroObject::OnDestroy()
{
	base::OnDestroy();

	//auto& pool = Scene::I->CreateObjectPool("bullet", 100, nullptr);

//for (auto& object : pool->GetMulti(100, true))
//{
//	object->SetPosition(mObject->GetPosition());
//}
}
