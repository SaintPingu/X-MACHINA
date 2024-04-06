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

	if (!target)
		return BT::NodeState::Running;

	const float kMinDistance = 0.1f;
	float distance = (mObject->GetPosition() - target->GetPosition()).Length();

	if (distance > kMinDistance) {
		if (mAStarAcctime >= 5.f) {
			PathPlanningAStar(target->GetPosition());
			mAStarAcctime = 0.f;
		}

		if (mPath.empty())
			return BT::NodeState::Running;
		
		Vec3 direction = (mPath.top() - mObject->GetPosition()).xz();

		mObject->RotateTargetAxisY(mPath.top(), mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(XMVector3Normalize(direction), mEnemyMgr->mMoveSpeed * DeltaTime());

		float l = direction.Length();
		if (l < 0.5f) {
			mPath.pop();
		}

		int currIndex = scene->GetGridIndexFromPos(mObject->GetPosition());
		Pos type = scene->GetTileIndexFromPos(mObject->GetPosition(), currIndex);
		printf("%d, %d\n", type.Z, type.X);
		//switch (type)
		//{
		//case TileObjectType::None:
		//	printf("None");
		//	break;
		//case TileObjectType::Static:
		//	printf("Static");
		//	break;
		//case TileObjectType::Dynamic:
		//	printf("Dynamic");
		//	break;
		//default:
		//	break;
		//}

		mEnemyMgr->mController->SetValue("Walk", true);
	}


	return BT::NodeState::Running;
}


void TaskMoveToTarget::PathPlanningAStar(const Vec3& targetPos)
{
	if (!mPath.empty())
		return;

	int currIndex = scene->GetGridIndexFromPos(mObject->GetPosition());
	Pos start = scene->GetTileIndexFromPos(mObject->GetPosition(), currIndex);
	Pos dest = scene->GetTileIndexFromPos(targetPos, currIndex);

	enum {
		DirCount = 8
	};

	Pos front[] = {
		Pos {-1, +0},	// UP
		Pos {+0, -1},	// LEFT
		Pos {+1, +0},	// DOWN
		Pos {+0, +1},	// RIGHT
		Pos {-1, -1},
		Pos {+1, -1},
		Pos {+1, +1},
		Pos {-1, +1},
	};

	INT32 cost[] = {
		10,
		10,
		10,
		10,
		14,
		14,
		14,
		14,
	};

	const INT32 size = 500;
	
	std::vector<std::vector<INT32>> dist(size, std::vector<int32>(size, INT_MAX));
	std::vector<std::vector<bool>> visited(size, std::vector<bool>(size, false));
	std::map<Pos, Pos> parent;

	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;
	INT32 g = 0;
	INT32 h = (abs(dest.Z - start.Z) + abs(dest.X - start.X)) * 10;
	pq.push({ g + h, g, start });
	dist[start.Z][start.X] = g + h;
	parent[start] = start;

	while (!pq.empty()) {
		PQNode curNode = pq.top();
		pq.pop();

		if (visited[curNode.Pos.Z][curNode.Pos.X])
			continue;

		visited[curNode.Pos.Z][curNode.Pos.X] = true;

		if (curNode.Pos == dest)
			break;

		for (int dir = 0; dir < DirCount; ++dir) {
			Pos nextPos = curNode.Pos + front[dir];

			if (nextPos.X >= 150 || nextPos.Z >= 150 || nextPos.X < 0 || nextPos.Z < 0)
				continue;

			if (scene->GetTileObjectTypeFromIndex(nextPos, currIndex) == TileObjectType::Static)
				continue;

			int g = curNode.G + cost[dir];
			int h = (abs(dest.Z - nextPos.Z) + abs(dest.X - nextPos.X)) * 10;

			if (dist[nextPos.Z][nextPos.X] > dist[curNode.Pos.Z][curNode.Pos.X] + cost[dir]) {
				dist[nextPos.Z][nextPos.X] = dist[curNode.Pos.Z][curNode.Pos.X] + cost[dir];
				pq.push({ g + h, g, nextPos });
				parent[nextPos] = curNode.Pos;
			}
		}
	}
	
	while (!mPath.empty())
		mPath.pop();

	Pos pos = dest;
	while (true) {
		mPath.push(scene->GetTilePosFromIndex(pos, currIndex));

		if (pos == parent[pos])
			break;

		pos = parent[pos];
	}
}
