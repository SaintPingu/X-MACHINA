#include "stdafx.h"
#include "ObjectMgr.h"

#include "Scene.h"
#include "Object.h"

#include "Script_ExplosiveObject.h"
#include "Script_Apache.h"
#include "Script_Gunship.h"

void ObjectMgr::InitObjectsScript()
{
	mObjects = Scene::I->GetAllObjects();
	for (rsptr<GameObject> object : mObjects) {
		switch (object->GetTag()) {
		case ObjectTag::ExplosiveBig:
		case ObjectTag::ExplosiveSmall:
			object->AddComponent<Script_ExplosiveObject>();

			break;
		case ObjectTag::Helicopter:
			// 모델의 이름에 따라 설정한다.
			switch (Hash(object->GetName())) {
			case Hash("Apache"):
				object->AddComponent<Script_Apache>();
				break;
			case Hash("Gunship"):
				object->AddComponent<Script_Gunship>();
				break;
			default:
				break;
			}

			break;
		default:
			break;
		}
	}
}
