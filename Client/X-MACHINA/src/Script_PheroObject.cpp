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

	auto& pool = Scene::I->CreateObjectPool("Prop_Book_C", 10, [&](rsptr<InstObject> object) {
		object->SetTag(ObjectTag::Dynamic);
		});

	for (auto& object : pool->GetMulti(10, true))
	{
		object->SetPosition(mObject->GetPosition());
	}
}
