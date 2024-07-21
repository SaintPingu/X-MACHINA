#include "stdafx.h"
#include "TaskPathPlanningAStar.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "BattleScene.h"
#include "Object.h"
#include "AnimatorController.h"
#include "MeshRenderer.h"
#include "Grid.h"


TaskPathPlanningAStar::TaskPathPlanningAStar(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mPath = &mEnemyMgr->mPath;
}


bool TaskPathPlanningAStar::PathPlanningAStar(Pos start, Pos dest)
{
	// �ʱ� ��ġ Ȥ�� ���� ������ Static�̶�� bfs�� ����� �ֺ� None ���� ȹ��
	if (BattleScene::I->GetTileFromUniqueIndex(start) == Tile::Static)
		start = FindNoneTileFromBfs(start);
	if (BattleScene::I->GetTileFromUniqueIndex(dest) == Tile::Static)
		dest = FindNoneTileFromBfs(dest);

	// �� �ʱ�ȭ 
	BattleScene::I->GetOpenList().clear();
	BattleScene::I->GetClosedList().clear();
	std::map<Pos, Pos>	mParent;
	std::map<Pos, int>	mDistance;
	std::map<Pos, bool>	mVisited;

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

		// ��ã�� ���� �� ������ ���� ���� ���� �������� ����
		if (mVisited.size() > mkMaxVisited) {
			dest = pq.top().Pos;
		}

		// �湮���� ���� ���鸸 �湮
		if (mVisited.contains(curNode.Pos))
			continue;

		if (mDistance[curNode.Pos] < curNode.F)
			continue;

		mVisited[curNode.Pos] = true;
		BattleScene::I->GetClosedList().push_back(BattleScene::I->GetTilePosFromUniqueIndex(curNode.Pos));

		// �ش� ������ �������� ��� ����
		if (curNode.Pos == dest)
			break;

		// 8�������� Ž��
		for (int dir = 0; dir < DirCount; ++dir) {
			Pos nextPos = curNode.Pos + gkFront[dir];

			// ���� ��ġ�� Ÿ���� ���� ������ ��� ��� continue
			if (abs(start.X - nextPos.X) > Grid::mTileRows || abs(start.Z - nextPos.Z) > Grid::mTileCols)
				continue;

			// ���� ���� ����� ���°� static�̶�� continue
			if (BattleScene::I->GetTileFromUniqueIndex(nextPos) == Tile::Static)
				continue;

			// �̹� �湮�� ���̸� continue
			if (mVisited.contains(nextPos))
				continue;

			// ���� �Ÿ� ������ ���ٸ� �Ÿ� ����� �ִ����� ����
			if (!mDistance.contains(nextPos))
				mDistance[nextPos] = INT32_MAX;

			// ��� ��� ������ 1 / 2
			int addCost{};
			if (prevDir != gkFront[dir])
				addCost = gkCost[0] / 2;

			int g = curNode.G + gkCost[dir] + addCost;
			int h = (abs(dest.Z - nextPos.Z) + abs(dest.X - nextPos.X)) * mkWeight;

			if (mDistance[nextPos] <= g + h)
				continue;

			mDistance[nextPos] = g + h;
			pq.push({ g + h, g, nextPos });
			mParent[nextPos] = curNode.Pos;
		}
	}

	Pos pos = dest;
	prevDir = { 0, 0 };

	while (!mEnemyMgr->mPath.empty())
		mEnemyMgr->mPath.pop();

	// �θ� ��θ� ���� ���ÿ� �־��ش�. top�� first path�̴�.
	while (true) {
		Pos dir = mParent[pos] - pos;

		if (prevDir != dir) {
			mPath->push(BattleScene::I->GetTilePosFromUniqueIndex(pos));
			BattleScene::I->GetOpenList().push_back(mPath->top());
		}

		if (pos == mParent[pos])
			break;

		pos = mParent[pos];
		prevDir = dir;
	}

	// �ڿ������� �������� ���� ù ��° ��δ� ��	��
	BattleScene::I->GetOpenList().push_back(BattleScene::I->GetTilePosFromUniqueIndex(start));
	if (!mPath->empty()) {
		mPath->pop();
	}

	if (mPath->empty()) {
		return false;
	}

	return true;
}

Pos TaskPathPlanningAStar::FindNoneTileFromBfs(const Pos& pos)
{
	std::queue<Pos> q;
	std::map<Pos, bool> visited;
	q.push(pos);

	Pos curPos{};
	while (!q.empty()) {
		curPos = q.front();
		q.pop();

		if (BattleScene::I->GetTileFromUniqueIndex(curPos) == Tile::None)
			return curPos;

		if (visited[curPos])
			continue;

		visited[curPos] = true;

		for (int dir = 0; dir < 4; ++dir) {
			Pos nextPos = curPos + gkFront[dir];
			q.push(nextPos);
		}
	}

	return curPos;
}
