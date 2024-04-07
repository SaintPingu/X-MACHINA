#include "stdafx.h"
#include "TaskMoveToTarget.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


TaskMoveToTarget::TaskMoveToTarget(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}


BT::NodeState TaskMoveToTarget::Evaluate()
{
	sptr<Object> target = GetData("target");
	mAStarAcctime += DeltaTime();

	// 타겟이 있을 때만 실행
	if (!target)
		return BT::NodeState::Running;

	// 오브젝트로부터 타겟까지의 벡터
	const float kMinDistance = 0.1f;
	float distanceToTarget = (mObject->GetPosition() - target->GetPosition()).Length();

	// 타겟에 도착하지 않았을 경우에만 이동
	if (distanceToTarget > kMinDistance) {
		if (scene->GetTileFromPos(mObject->GetLook() * Grid::mkTileHeight + mObject->GetPosition()) == Tile::Static || !mPath.empty()) {
			if (mAStarAcctime >= 5.f) {
				if (false == PathPlanningAStar(target->GetPosition()))
					return BT::NodeState::Failure;

				mAStarAcctime = 0.f;
			}

			// 결과인 경로가 비었다면 리턴
			if (mPath.empty())
				return BT::NodeState::Running;

			// 첫 번째 경로까지 이동 및 회전
			Vec3 toFirstPath = (mPath.top() - mObject->GetPosition()).xz();
			mObject->RotateTargetAxisY(mPath.top(), mEnemyMgr->mRotationSpeed * DeltaTime());
			mObject->Translate(XMVector3Normalize(toFirstPath), mEnemyMgr->mMoveSpeed * DeltaTime());

			// 첫 번째 경로까지 도착했다면 첫 번째 경로 제거
			if (toFirstPath.Length() < kMinDistance) {
				mPath.pop();
			}
		}
		else {
			mObject->RotateTargetAxisY(target->GetPosition(), mEnemyMgr->mRotationSpeed * DeltaTime());
			mObject->Translate(mObject->GetLook(), mEnemyMgr->mMoveSpeed * DeltaTime());
		}
	}

	return BT::NodeState::Running;
}


bool TaskMoveToTarget::PathPlanningAStar(const Vec3& targetPos)
{
	// 시작 지점과 목적지 위치 값을 타일 고유 인덱스로 변환
	Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
	Pos dest = scene->GetTileUniqueIndexFromPos(targetPos);

	// 목적지가 막혀 있다면 초기에 리턴
	if (scene->GetTileFromUniqueIndex(dest) == Tile::Static)
		return false;

	// 상하좌우 대각선까지 8방향
	enum {
		DirCount = 8
	};

	Pos front[] = {
		Pos {+1, +0},	// UP
		Pos {+0, -1},	// LEFT
		Pos {-1, +0},	// DOWN
		Pos {+0, +1},	// RIGHT
		Pos {-1, +1},
		Pos {+1, +1},
		Pos {+1, -1},
		Pos {-1, -1},
	};

	int cost[] = {
		10,
		10,
		10,
		10,
		14,
		14,
		14,
		14,
	};

	// 값 초기화 
	mParent.clear();
	mDistance.clear();
	mVisited.clear();
	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;
	constexpr int kWeight = 10;

	// f = g + h
	int g = 0;
	int h = (abs(dest.Z - start.Z) + abs(dest.X - start.X)) * kWeight;
	pq.push({ g + h, g, start });
	mDistance[start] = g + h;
	mParent[start] = start;

	// AStar 실행
	while (!pq.empty()) {
		PQNode curNode = pq.top();
		pq.pop();

		// 방문하지 않은 노드들만 방문
		if (mVisited.contains(curNode.Pos))
			continue;

		if (mDistance[curNode.Pos] < curNode.F)
			continue;

		mVisited[curNode.Pos] = true;

		// 해당 지점이 목적지인 경우 종료
		if (curNode.Pos == dest)
			break;

		// 8방향으로 탐색
		for (int dir = 0; dir < DirCount; ++dir) {
			Pos nextPos = curNode.Pos + front[dir];

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
			int g = curNode.G + cost[dir];
			int h = (abs(dest.Z - nextPos.Z) + abs(dest.X - nextPos.X)) * kWeight;

			if (mDistance[nextPos] <= g + h)
				continue;

			mDistance[nextPos] = g + h;
			pq.push({ g + h, g, nextPos });
			mParent[nextPos] = curNode.Pos;
		}
	}

	// 경로 비우기
	while (!mPath.empty())
		mPath.pop();

	// 부모 경로를 따라가 스택에 넣어준다. top이 first path이다.
	Pos pos = dest;
	while (true) {
		mPath.push(scene->GetTilePosFromUniqueIndex(pos));

		if (pos == mParent[pos])
			break;

		pos = mParent[pos];
	}

	return true;
}
