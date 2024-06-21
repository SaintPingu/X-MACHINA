#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region Class
class Script_AfterImageObject : public Component {
	COMPONENT(Script_AfterImageObject, Component)

private:
	static constexpr UINT mkMaxCreateCnt = 50;

	float mAccTime{};

	UINT mRateOverCreateCnt{};
	float mAfterImageLifeTime{};
	
	UINT mCurrObjectIndex{};
	std::deque<sptr<class GameObject>> mAfterImageObjects{};

public:
	void SetAfterImage(UINT createCnt, float lifeTime);

public:
	virtual void OnEnable() override;
	virtual void Awake() override;
	virtual void Update() override;
	
private:
	void PushObject();
	void PopObject();
};
#pragma endregion

