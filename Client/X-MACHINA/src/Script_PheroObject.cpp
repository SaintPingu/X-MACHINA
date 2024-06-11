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
	auto& pheroPool = Scene::I->CreateObjectPool("Level3Phero", mCreateCnt, [&](rsptr<InstObject> object) {
		object->SetTag(ObjectTag::Dynamic);
		});
	
	for (auto& phero : pheroPool->GetMulti(mCreateCnt, true))
	{
		phero->SetPosition(mObject->GetPosition());
		phero->AddComponent<Script_Phero>()->SetPheroStat(1);
	}


	base::OnDestroy();
}
