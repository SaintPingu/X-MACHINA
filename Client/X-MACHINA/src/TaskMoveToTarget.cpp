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

		mEnemyMgr->mController->SetValue("Walk", true);
	}


	return BT::NodeState::Running;
}


void TaskMoveToTarget::PathPlanningAStar(const Vec3& targetPos)
{
	if (!mPath.empty())
		return;

	mParent.clear();
	mDist.clear();
	mVisited.clear();

	Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
	Pos dest = scene->GetTileUniqueIndexFromPos(targetPos);

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

	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;
	INT32 g = 0;
	INT32 h = (abs(dest.Z - start.Z) + abs(dest.X - start.X)) * 10;
	pq.push({ g + h, g, start });
	mDist[start] = g + h;
	mParent[start] = start;

	while (!pq.empty()) {
		PQNode curNode = pq.top();
		pq.pop();

		if (mVisited.size() > mMaxVisited)
			return;

		if (curNode.Pos == dest)
			break;

		if (mVisited.find(curNode.Pos) != mVisited.end())
			continue;

		mVisited[curNode.Pos] = true;

		for (int dir = 0; dir < DirCount; ++dir) {
			Pos nextPos = curNode.Pos + front[dir];

			if (scene->GetTileObjectTypeFromUniqueIndex(nextPos) == TileObjectType::Static)
				continue;

			int g = curNode.G + cost[dir];
			int h = (abs(dest.Z - nextPos.Z) + abs(dest.X - nextPos.X)) * 10;

			if (mDist.find(nextPos) == mDist.end())
				mDist[nextPos] = INT32_MAX;

			if (mDist[nextPos] > mDist[curNode.Pos] + cost[dir]) {
				mDist[nextPos] = mDist[curNode.Pos] + cost[dir];
				pq.push({ g + h, g, nextPos });
				mParent[nextPos] = curNode.Pos;
			}
		}
	}
	
	while (!mPath.empty())
		mPath.pop();

	Pos pos = dest;
	while (true) {
		mPath.push(scene->GetTilePosFromUniqueIndex(pos));

		if (pos == mParent[pos])
			break;

		pos = mParent[pos];
	}
}
