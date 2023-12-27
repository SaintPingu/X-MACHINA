#include "stdafx.h"
#include "Script_Player.h"
#include "Object.h"
#include "Scene.h"
#include "Rigidbody.h"
#include "UI.h"

static BoundingBox border = { Vec3(256, 100, 256), Vec3(2400, 1100, 2400) };



void Script_Player::SetSpawn(const Vec3& pos)
{
	mObject->SetPosition(pos);
	XMStoreFloat4x4(&mSpawnTransform, _MATRIX(mObject->GetWorldTransform()));
}

void Script_Player::SetAcc(float acc)
{
	mAcc = acc;
}

void Script_Player::SetMaxSpeed(float speed)
{
	mRigid->SetMaxSpeed(speed);
}

void Script_Player::Start()
{
	mObject->SetTag(ObjectTag::Player);
	mPlayer = mObject->GetObj<GameObject>();
	mRigid = mObject->AddComponent<Rigidbody>();
}

void Script_Player::Update()
{
	if (!border.Contains(_VECTOR(mObject->GetPosition()))) {
		mObject->ReturnTransform();
	}
}

void Script_Player::Move(DWORD dwDirection)
{
	if (mPlayerType == PlayerType::Tank) {
		dwDirection &= ~DIR_UP;
		dwDirection &= ~DIR_DOWN;
	}

	//mPlayer->SetMovingDirection(mObject->GetDirection(dwDirection));
	Vec3 force = Vector3::Multiply(mObject->GetDirection(dwDirection), mAcc);
	mRigid->AddForce(force);
}

void Script_Player::Rotate(float x, float y, float z)
{
	mObject->Rotate(x, y, z);
}

void Script_Player::Respawn()
{
	mObject->SetWorldTransform(mSpawnTransform);
	mRigid->Stop();
	mObject->Update();
}

void Script_Player::Explode()
{
	crntScene->CreateExplosion(Scene::ExplosionType::Big, mObject->GetPosition());
	Respawn();
}

void Script_Player::Hit(float damage)
{
	if (mHP <= 0) {
		return;
	}

	mHP -= damage;
	if (mHP <= 0) {
		Explode();
	}
}

void Script_Player::AddScore(int score)
{
	mScore += score;
	Canvas::Inst()->SetScore(mScore);
}