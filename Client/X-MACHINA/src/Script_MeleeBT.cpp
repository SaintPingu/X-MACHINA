#include "stdafx.h"
#include "Script_MeleeBT.h"

#include "CheckDetectionRange.h"
#include "CheckAttackRange.h"
#include "TaskPatrol.h"
#include "TaskMoveToTarget.h"
#include "TaskAttack.h"
#include "TaskPathPlanning.h"

BT::Node* Script_MeleeBT::SetupTree()
{
	// ������Ʈ ���� ��ġ �߽� (5, 5) ��ŭ ���� ��ġ ����
	std::vector<Vec3> wayPoints(1);
	wayPoints[0] = mObject->GetLocalPosition() + Vec3(0.f, 0.f, 0.f);
	//wayPoints[0] = mObject->GetLocalPosition() + Vec3(5.f, 0.f, 5.f);
	//wayPoints[1] = mObject->GetLocalPosition() + Vec3(5.f, 0.f, -5.f);
	//wayPoints[2] = mObject->GetLocalPosition() + Vec3(-5.f, 0.f, -5.f);
	//wayPoints[3] = mObject->GetLocalPosition() + Vec3(-5.f, 0.f, 5.f);
	//
	// �ൿ Ʈ�� ����
	BT::Node* root = new BT::Selector{ std::vector<BT::Node*>{
		new BT::Sequence{ std::vector<BT::Node*>{
			new CheckAttackRange(mObject),
			new TaskAttack(mObject),
		}},
		new BT::Sequence{ std::vector<BT::Node*>{
			new CheckDetectionRange(mObject),
			new BT::Selector { std::vector<BT::Node*>{
				new TaskMoveToTarget(mObject),
				new TaskPathPlanning(mObject),
			}},
		}},
		new TaskPatrol {mObject, std::move(wayPoints)}
	}};

	return root;
}
