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

	// 일정 시간 후 경로 재탐색
	if (mAccTime > 3.f) {
		mAccTime = 0.f;
		mEnemyMgr->mIsMoveToPath = false;
		while (!mPath.empty()) mPath.pop();
		return BT::NodeState::Success;
	}

	// 이전에 경로 이동이 취소 되었다면 모두 비워야함
	if (!mEnemyMgr->mIsMoveToPath)
		while (!mPath.empty()) mPath.pop();

	// 이미 찾은 길이 있는 경우 해당 길을 찾아 이동 및 회전
	if (!mPath.empty()) {
		mAccTime += DeltaTime();
		Vec3 toFirstPath = (mPath.top() - mObject->GetPosition()).xz();
		mObject->RotateTargetAxisY(mPath.top(), mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(XMVector3Normalize(toFirstPath), mEnemyMgr->mMoveSpeed * DeltaTime());

		if (toFirstPath.Length() < kMinDistance)
			mPath.pop();

		if (mPath.empty())
			mEnemyMgr->mIsMoveToPath = false;

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
	// 값 초기화 
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

	Pos pos = dest;

	// AStar 실행
	int cnt{};
	Pos prevDir;
	while (!pq.empty()) {
		PQNode curNode = pq.top();
		prevDir = curNode.Pos - mParent[curNode.Pos];
		pq.pop();

		if (mkMaxVisited < cnt) {
			pos = curNode.Pos;
			break;
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
			Pos nextPos = curNode.Pos + kFront[dir];
			
			// 다음 방향 노드의 상태가 static이라면 continue
			if (scene->GetTileFromUniqueIndex(nextPos) == Tile::Static)
				continue;

			// 이미 방문한 곳이면 continue
			if (mVisited.contains(nextPos))
				continue;

			// 현재 거리 정보가 없다면 거리 비용을 최댓값으로 설정
			if (!mDistance.contains(nextPos))
				mDistance[nextPos] = INT32_MAX;

			// 비용 계산
			int addCost{};
			if (prevDir != kFront[dir])
				addCost = kCost[dir] / 2;

			int g = curNode.G + kCost[dir] + addCost;
			int h = (abs(dest.Z - nextPos.Z) + abs(dest.X - nextPos.X)) * mkWeight;

			if (mDistance[nextPos] <= g + h)
				continue;

			mDistance[nextPos] = g + h;
			pq.push({ g + h, g, nextPos });
			mParent[nextPos] = curNode.Pos;
		}

		cnt++;
	}

	// 부모 경로를 따라가 스택에 넣어준다. top이 first path이다.
	while (true) {
		mPath.push(scene->GetTilePosFromUniqueIndex(pos));
		scene->GetOpenList().push_back(mPath.top());

		if (pos == mParent[pos])
			break;

		pos = mParent[pos];
	}

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
