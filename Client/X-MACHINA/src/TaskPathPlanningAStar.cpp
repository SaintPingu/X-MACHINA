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
	// 초기 위치 혹은 도착 지점이 Static이라면 bfs를 사용해 주변 None 지점 획득
	if (BattleScene::I->GetTileFromUniqueIndex(start) == Tile::Static)
		start = FindNoneTileFromBfs(start);
	if (BattleScene::I->GetTileFromUniqueIndex(dest) == Tile::Static)
		dest = FindNoneTileFromBfs(dest);

	// 값 초기화 
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

	// AStar 실행
	Pos prevDir;
	while (!pq.empty()) {
		PQNode curNode = pq.top();
		prevDir = curNode.Pos - mParent[curNode.Pos];
		pq.pop();

		// 길찾기 실패 시 점수가 가장 높은 곳을 도착지로 설정
		if (mVisited.size() > mkMaxVisited) {
			dest = pq.top().Pos;
		}

		// 방문하지 않은 노드들만 방문
		if (mVisited.contains(curNode.Pos))
			continue;

		if (mDistance[curNode.Pos] < curNode.F)
			continue;

		mVisited[curNode.Pos] = true;
		BattleScene::I->GetClosedList().push_back(BattleScene::I->GetTilePosFromUniqueIndex(curNode.Pos));

		// 해당 지점이 목적지인 경우 종료
		if (curNode.Pos == dest)
			break;

		// 8방향으로 탐색
		for (int dir = 0; dir < DirCount; ++dir) {
			Pos nextPos = curNode.Pos + gkFront[dir];

			// 다음 위치의 타일이 일정 범위를 벗어난 경우 continue
			if (abs(start.X - nextPos.X) > Grid::mTileRows || abs(start.Z - nextPos.Z) > Grid::mTileCols)
				continue;

			// 다음 방향 노드의 상태가 static이라면 continue
			if (BattleScene::I->GetTileFromUniqueIndex(nextPos) == Tile::Static)
				continue;

			// 이미 방문한 곳이면 continue
			if (mVisited.contains(nextPos))
				continue;

			// 현재 거리 정보가 없다면 거리 비용을 최댓값으로 설정
			if (!mDistance.contains(nextPos))
				mDistance[nextPos] = INT32_MAX;

			// 비용 계산 보통의 1 / 2
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

	// 부모 경로를 따라가 스택에 넣어준다. top이 first path이다.
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

	// 자연스러운 움직임을 위해 첫 번째 경로는 삭	제
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
