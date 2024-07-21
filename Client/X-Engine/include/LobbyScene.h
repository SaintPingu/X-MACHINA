#pragma once

class UI;

class LobbyScene : public Singleton<LobbyScene> {
	friend Singleton;

public:
	std::vector<sptr<UI>> mUIs{};

public:
	void Init();
	void Update();
	void Render();
};
