#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Canvas;
class Button;
class Texture;
class Shader;
class ModelObjectMesh;
#pragma endregion



#pragma region Class
// 2D object (not entity)
class UI : public Object {
	using base = Object;
	using Object::Destroy;

protected:
	bool mIsHoverable{};
	bool mIsHover{};
	sptr<Texture> mTexture{};
	sptr<Shader>  mShader{};

	Vec2			mScale{};

	std::function<void()> mClickCallback{};
	std::string mClickSound{};
	std::string mHoverSound{};

public:
	// [texture]를 설정하고, [pos]위치에 [width * height] 크기의 UI를 생성한다.
	UI(const std::string& textureName, const Vec2& pos = Vec2::Zero, Vec2 scale = Vec2::Zero);
	virtual ~UI() = default;

public:
	virtual void Update() { base::Update(); }
	virtual void Render();
	virtual void Remove();

	sptr<Texture> GetTexture() const { return mTexture; }
	Vec2 GetScale() const;
	bool IsHover() const { return mIsHover; }

	void SetPosition(float x, float y);
	void SetPosition(const Vec2& pos);
	void SetPosition(const Vec3& pos);

	void SetScale(const Vec2& scale);

	void RemoveColor();
	void SetColor(const Vec3& color);
	virtual void SetHover(bool val);
	void SetHoverable(bool val) { mIsHoverable = val; }
	void SetOpacity(float val);
	void SetShader(sptr<Shader> shader) { mShader = shader; }

public:
	void ChangeTexture(rsptr<Texture> texture) { mTexture = texture; }
	bool CheckHover() const;
	virtual void OnClick();
	virtual void OffClick() {}

	void SetClickCallback(const std::function<void()> callback) { mClickCallback = callback; }
	void SetClickSound(const std::string& sound) { mClickSound = sound; }
	void SetHoverSound(const std::string& sound) { mHoverSound = sound; }

	virtual void ProcessKeyboardMsg(UINT message, WPARAM wParam, LPARAM lParam) {}

protected:
	virtual void UpdateShaderVars(rsptr<Texture> texture);
};


class SliderUI : public UI {
private:
	float mMinValue = 0.f;
	float mMaxValue = 1.f;
	float mValue{};			// 0~1 normalize value

public:
	SliderUI(const std::string& textureName, const Vec2& pos = Vec2::Zero, Vec2 scale = Vec2::Zero);
	virtual ~SliderUI() = default;

public:
	void SetMinMaxValue(float min = 0.f, float max = 1.f) { mMinValue = min, mMaxValue = max; }
	void SetValue(float value) { mValue = value / mMaxValue; }
	void SetDissolve(const Vec3& color);

protected:
	virtual void UpdateShaderVars(rsptr<Texture> texture) override;
};


class Button : public UI {
	using base = UI;

private:
	bool mClicked{};
	sptr<Texture> mCrntTexture{};
	sptr<Texture> mHighlightTexture{};
	sptr<Texture> mPressedTexture{};
	sptr<Texture> mDisabledTexture{};

public:
	Button(const std::string& textureName, const Vec2& pos = Vec2::Zero, Vec2 scale = Vec2::Zero);

public:
	virtual void Update() override;
	virtual void Render() override;

public:
	void SetHighlightTexture(const std::string& textureName);
	void SetPressedTexture(const std::string& textureName);
	void SetDisabledTexture(const std::string& textureName);

protected:
	virtual void UpdateShaderVars(rsptr<Texture> texture) override;

private:
	virtual void OnClick() override;
};

class InputField : public UI {
	using base = UI;

private:
	bool mClicked{};

	std::wstring	mText{};
	std::wstring	mImeCompositionString = L"";

	std::size_t		mLastChatIdx{};

public:
	virtual void ProcessKeyboardMsg(UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	virtual void OnClick() override;
	virtual void OffClick() override;
};

// Canvas 위에 UI를 그리도록 한다.
class Canvas : public Singleton<Canvas> {
	friend Singleton;
	using Layer = int;

	template <typename T>
	static constexpr bool is_valid_ui_type = (std::is_same<T, UI>::value || std::is_same<T, SliderUI>::value || std::is_same<T, Button>::value);

private:
	static constexpr UINT8 mkLayerCnt = 10;
	std::array<std::unordered_set<sptr<UI>>, mkLayerCnt> mUIs{}; // all UIs

	float mWidth{};
	float mHeight{};

private:
	Canvas() = default;
	virtual ~Canvas() = default;

public:
	float GetWidth() const { return mWidth; }
	float GetHeight() const { return mHeight; }

public:
	void Init();

	void Update();
	void Render() const;
	void Clear();

	template<class T, typename std::enable_if<is_valid_ui_type<T>>::type* = nullptr>
	T* CreateUI(Layer layer, const std::string& texture, const Vec2& pos = Vec2::Zero, Vec2 scale = Vec2::Zero)
	{
		if (layer > (mkLayerCnt - 1))
			return nullptr;

		sptr<T> ui = std::make_shared<T>(texture, pos, scale);
		ui->SetActive(true);
		mUIs[layer].insert(ui);
		return ui.get();
	}
	
	// should make nullptr [ui]
	void RemoveUI(UI* ui);
	void RemoveUI(Layer layer, UI* ui);

	void CheckClick() const;
	void CheckHover() const;

	void ProcessActiveUI(std::function<void(sptr<UI>)> processFunc, bool isReverse = false) const;
	void ProcessAllUI(std::function<void(sptr<UI>)> processFunc, bool isReverse = false) const;

	void ProcessKeyboardMsg(UINT message, WPARAM wParam, LPARAM lParam);
};
#pragma endregion
