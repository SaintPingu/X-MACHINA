#include "stdafx.h"
#include "Script_EnemyNetwork.h"

void Script_EnemyNetwork::Awake()
{
}

void Script_EnemyNetwork::LateUpdate()
{
}

void Script_EnemyNetwork::UpdateData(const void* data)
{
}

void Script_EnemyNetwork::SetPostion(const Vec3& pos)
{
	mObject->SetPosition(pos);
}

void Script_EnemyNetwork::SetRotation(const Vec3& rot)
{
	mObject->SetLocalRotation(Quaternion::ToQuaternion(rot));
}
