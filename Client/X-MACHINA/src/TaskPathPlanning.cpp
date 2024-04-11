#include "stdafx.h"
#include "TaskPathPlanning.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"
#include "MeshRenderer.h"
#include "Grid.h"


TaskPathPlanning::TaskPathPlanning(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}


BT::NodeState TaskPathPlanning::Evaluate()
{
	sptr<Object> target = GetData("target");

	// 이전에 경로 이동이 취소 되었다면 모두 비워야함
	if (!mEnemyMgr->mIsMoveToPath)
		while (!mPath.empty()) mPath.pop();

	// 이미 찾은 길이 있는 경우 해당 길을 찾아 이동 및 회전
	if (!mPath.empty()) {
		Vec3 toFirstPath = (mPath.top() - mObject->GetPosition()).xz();
		mObject->RotateTargetAxisY(mPath.top(), mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(XMVector3Normalize(toFirstPath), mEnemyMgr->mMoveSpeed * DeltaTime());

		const float kMinDistance = 0.1f;
		if (toFirstPath.Length() < kMinDistance)
			mPath.pop();

		return BT::NodeState::Success;
	}
	
	// 시작 지점과 목적지 위치 값을 타일 고유 인덱스로 변환
	Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
	Pos dest = scene->GetTileUniqueIndexFromPos(target->GetPosition());

	// 경로 계획에 실패했다면 Failure를 호출하여 다음 노드로 넘어감
	if (PathPlanningAStar(start, dest) == false)
		return BT::NodeState::Failure;

	return BT::NodeState::Success;
}

bool TaskPathPlanning::PathPlanningAStar(Pos start, Pos dest)
{
	// 초기 위치가 Static이라면 길찾기를 하지 않는다.
	if (scene->GetTileFromUniqueIndex(start) == Tile::Static)
		return false;

	// 값 초기화 
	scene->GetOpenList().clear();
	scene->GetClosedList().clear();
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
		scene->GetClosedList().push_back(scene->GetTilePosFromUniqueIndex(curNode.Pos));

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
			if (scene->GetTileFromUniqueIndex(nextPos) == Tile::Static)
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
	prevDir = { 0, 0 };

	// 부모 경로를 따라가 스택에 넣어준다. top이 first path이다.
	while (true) {
		Pos dir = mParent[pos] - pos;

		if (prevDir != dir)
			mPath.push(scene->GetTilePosFromUniqueIndex(pos));

		scene->GetOpenList().push_back(mPath.top());

		if (pos == mParent[pos])
			break;

		pos = mParent[pos];
		prevDir = dir;
	}

	if (mPath.empty()) {
		mEnemyMgr->mIsMoveToPath = false;
		return false;
	}

	mEnemyMgr->mIsMoveToPath = true;
	return true;
}
