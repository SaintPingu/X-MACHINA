#include "stdafx.h"
#include "Script_PheroObject.h"

#include "Scene.h"
#include "ObjectPool.h"
#include "Object.h"
#include "Script_Phero.h"

void Script_PheroObject::Start()
{
	base::Start();
	
	mCreateCnt = Math::RandInt(0, 10);
}

void Script_PheroObject::OnDestroy()
{
	base::OnDestroy();

	auto& pool = Scene::I->CreateObjectPool("Phero", mCreateCnt, [&](rsptr<InstObject> object) {
		object->SetTag(ObjectTag::Dynamic);
		});
	
	for (auto& object : pool->GetMulti(mCreateCnt, true))
	{
		object->AddComponent<Script_Phero>();

		const Vec3 pos = mObject->GetPosition() + Vec3{ Math::RandFloat(-2.f, 2.f), 1.f, Math::RandFloat(-2.f, 2.f) };
		object->SetPosition(pos);
	}
}
