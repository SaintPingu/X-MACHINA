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

	// Ÿ���� ���� ���� ����
	if (!target)
		return BT::NodeState::Running;

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	const float kMinDistance = 0.1f;
	float distanceToTarget = (mObject->GetPosition() - target->GetPosition()).Length();

	// Ÿ�ٿ� �������� �ʾ��� ��쿡�� �̵�
	if (distanceToTarget > kMinDistance) {

		// AStar�� ���� �ð� �������� �ٽ� ����
		if (mAStarAcctime >= 5.f) {
			if (false == PathPlanningAStar(target->GetPosition()))
				return BT::NodeState::Running;

			mAStarAcctime = 0.f;
		}

		// ����� ��ΰ� ����ٸ� ����
		if (mPath.empty())
			return BT::NodeState::Running;
		
		// ù ��° ��α��� �̵� �� ȸ��
		Vec3 toFirstPath = (mPath.top() - mObject->GetPosition()).xz();
		mObject->RotateTargetAxisY(mPath.top(), mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(XMVector3Normalize(toFirstPath), mEnemyMgr->mMoveSpeed * DeltaTime());
		
		// ù ��° ��α��� �����ߴٸ� ù ��° ��� ����
		if (toFirstPath.Length() < kMinDistance) {
			mPath.pop();
		}
	}

	return BT::NodeState::Running;
}


bool TaskMoveToTarget::PathPlanningAStar(const Vec3& targetPos)
{
	// ���� ������ ������ ��ġ ���� Ÿ�� ���� �ε����� ��ȯ
	Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
	Pos dest = scene->GetTileUniqueIndexFromPos(targetPos);

	// �������� ���� �ִٸ� �ʱ⿡ ����
	if (scene->GetTileFromUniqueIndex(dest) == Tile::Static)
		return false;

	// �����¿� �밢������ 8����
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

	// �� �ʱ�ȭ 
	mParent.clear();
	mDist.clear();
	mVisited.clear();
	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;
	constexpr int kWeight = 10;

	// f = g + h
	int g = 0;
	int h = static_cast<int>(sqrtf(powf((float)dest.X - (float)start.X, 2) + powf((float)dest.X - (float)start.X, 2))) * kWeight;
	pq.push({ g + h, g, start });
	
	mDist[start] = g + h;
	mParent[start] = start;

	// AStar ����
	while (!pq.empty()) {
		PQNode curNode = pq.top();
		pq.pop();

		// �ش� ������ �������� ��� ����
		if (curNode.Pos == dest)
			break;

		// �湮���� ���� ���鸸 �湮
		if (mVisited.find(curNode.Pos) != mVisited.end())
			continue;

		mVisited[curNode.Pos] = true;

		// 8�������� Ž��
		for (int dir = 0; dir < DirCount; ++dir) {
			Pos nextPos = curNode.Pos + front[dir];

			// ���� ���� ����� ���°� static�̶�� continue
			if (scene->GetTileFromUniqueIndex(nextPos) == Tile::Static)
				continue;

			// ù �湮�̶�� �ִ����� ����
			if (mDist.find(nextPos) == mDist.end())
				mDist[nextPos] = INT32_MAX;

			// ��� ���
			int g = curNode.G + cost[dir];
			int h = static_cast<int>(sqrtf(powf((float)dest.X - (float)start.X, 2) + powf((float)dest.X - (float)start.X, 2))) * kWeight;

			// �Ÿ� �� ������Ʈ
			if (mDist[nextPos] > mDist[curNode.Pos] + cost[dir]) {
				mDist[nextPos] = mDist[curNode.Pos] + cost[dir];
				pq.push({ g + h, g, nextPos });
				mParent[nextPos] = curNode.Pos;
			}
		}
	}
	
	// ��� ����
	while (!mPath.empty())
		mPath.pop();

	// �θ� ��θ� ���� ���ÿ� �־��ش�. top�� first path�̴�.
	Pos pos = dest;
	while (true) {
		mPath.push(scene->GetTilePosFromUniqueIndex(pos));

		if (pos == mParent[pos])
			break;

		pos = mParent[pos];
	}

	return true;
}
