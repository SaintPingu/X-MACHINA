#pragma once

#include "Script_NetworkWeapon.h"


class Script_NetworkBulletWeapon : public Script_NetworkWeapon {
	COMPONENT(Script_NetworkBulletWeapon, Script_NetworkWeapon)

public:
	enum class BulletType { Bullet, Missile };

protected:
	Vec2 mErrX{};	// �¿� ����
	Vec2 mErrY{};	// ���� ����
	float mSpeerErr{};	// �ӵ� ����

protected:
	virtual void FireBullet() override;

	void InitBullet(rsptr<InstObject> bullet, float damage, float speed, BulletType bulletType = BulletType::Bullet) const;

private:
	virtual void CreateBulletPool() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const abstract;


};

