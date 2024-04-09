#include "stdafx.h"
#include "TaskPathPlanning.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"
#include "MeshRenderer.h"


namespace {
	enum {
		DirCount = 8
	};

	Pos kFront[] = {
		Pos {+1, +0},
		Pos {+0, -1},
		Pos {-1, +0},
		Pos {+0, +1},
		Pos {-1, +1},
		Pos {+1, +1},
		Pos {+1, -1},
		Pos {-1, -1},
	};

	int kCost[] = {
		10,
		10,
		10,
		10,
		14,
		14,
		14,
		14,
	};
}


TaskPathPlanning::TaskPathPlanning(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}


BT::NodeState TaskPathPlanning::Evaluate()
{
	sptr<Object> target = GetData("target");

	const float kMinDistance = 0.1f;

	// ������ ��� �̵��� ��� �Ǿ��ٸ� ��� �������
	if (!mEnemyMgr->mIsMoveToPath)
		while (!mPath.empty()) 
			mPath.pop();

	// �̹� ã�� ���� �ִ� ��� �ش� ���� ã�� �̵� �� ȸ��
	if (!mPath.empty()) {
		Vec3 toFirstPath = (mPath.top() - mObject->GetPosition()).xz();
		mObject->RotateTargetAxisY(mPath.top(), mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(XMVector3Normalize(toFirstPath), mEnemyMgr->mMoveSpeed * DeltaTime());

		if (toFirstPath.Length() < kMinDistance)
			mPath.pop();

		return BT::NodeState::Success;
	}
	
	// ���� ������ ������ ��ġ ���� Ÿ�� ���� �ε����� ��ȯ
	Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
	Pos dest = scene->GetTileUniqueIndexFromPos(target->GetPosition());

	// ��� ��ȹ�� �����ߴٸ� Failure�� ȣ���Ͽ� ���� ���� �Ѿ
	if (PathPlanningAStar(start, dest) == false)
		return BT::NodeState::Failure;

	return BT::NodeState::Success;
}

bool TaskPathPlanning::PathPlanningAStar(Pos start, Pos dest)
{
	// �� �ʱ�ȭ 
	scene->GetOpenList().clear();
	scene->GetClosedList().clear();
	mParent.clear();
	mDistance.clear();
	mVisited.clear();

	// f = g + h
	int g = 0;
	int h = (abs(dest.Z - start.Z) + abs(dest.X - start.X)) * mkWeight;
	pq.push({ g + h, g, start });
	mDistance[start] = g + h;
	mParent[start] = start;

	// AStar ����
	Pos prevDir;
	while (!pq.empty()) {
		PQNode curNode = pq.top();
		prevDir = curNode.Pos - mParent[curNode.Pos];
		pq.pop();

		// �湮���� ���� ���鸸 �湮
		if (mVisited.contains(curNode.Pos))
			continue;

		if (mDistance[curNode.Pos] < curNode.F)
			continue;

		mVisited[curNode.Pos] = true;
		scene->GetClosedList().push_back(scene->GetTilePosFromUniqueIndex(curNode.Pos));

		// �ش� ������ �������� ��� ����
		if (curNode.Pos == dest)
			break;

		// 8�������� Ž��
		for (int dir = 0; dir < DirCount; ++dir) {
			Pos nextPos = curNode.Pos + kFront[dir];
			
			// ���� ��ġ�� Ÿ���� ���� ������ ��� ��� continue
			if (abs(start.X - nextPos.X) > Grid::mTileRows || abs(start.Z - nextPos.Z) > Grid::mTileCols)
				continue;

			// ���� ���� ����� ���°� static�̶�� continue
			if (scene->GetTileFromUniqueIndex(nextPos) == Tile::Static)
				continue;

			// �̹� �湮�� ���̸� continue
			if (mVisited.contains(nextPos))
				continue;

			// ���� �Ÿ� ������ ���ٸ� �Ÿ� ����� �ִ����� ����
			if (!mDistance.contains(nextPos))
				mDistance[nextPos] = INT32_MAX;

			// ��� ��� ������ 1 / 2
			int addCost{};
			if (prevDir != kFront[dir])
				addCost = kCost[0] / 2;

			int g = curNode.G + kCost[dir] + addCost;
			int h = (abs(dest.Z - nextPos.Z) + abs(dest.X - nextPos.X)) * mkWeight;

			if (mDistance[nextPos] <= g + h)
				continue;

			mDistance[nextPos] = g + h;
			pq.push({ g + h, g, nextPos });
			mParent[nextPos] = curNode.Pos;
		}
	}

	Pos pos = dest;

	// �θ� ��θ� ���� ���ÿ� �־��ش�. top�� first path�̴�.
	while (true) {
		mPath.push(scene->GetTilePosFromUniqueIndex(pos));
		scene->GetOpenList().push_back(mPath.top());

		if (pos == mParent[pos])
			break;

		pos = mParent[pos];
	}

	// �ڿ������� �������� ���� �ʹ� �� ��δ� ����
	for (int i = 0; i < mkPathAdjust; ++i) {
		if (mPath.empty())
			break;

		mPath.pop();
	}

	if (mPath.empty()) {
		mEnemyMgr->mIsMoveToPath = false;
		return false;
	}

	mEnemyMgr->mIsMoveToPath = true;
	return true;
}
