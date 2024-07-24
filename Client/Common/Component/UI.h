#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Canvas;
class Texture;
class Shader;
class ModelObjectMesh;
#pragma endregion



#pragma region Class
// 2D object (not entity)
class UI : public Object {
	using base = Object;

protected:
	bool		  mIsActive = true;
	sptr<Texture> mTexture{};
	sptr<Shader>  mShader{};

	Vec2			mScale{};

	std::function<void()> mClickCallback{};

public:
	// [texture]를 설정하고, [pos]위치에 [width * height] 크기의 UI를 생성한다.
	UI(const std::string& textureName, const Vec2& pos, const Vec2& scale);
	virtual ~UI() = default;

public:
	virtual void Update() {}
	virtual void Render();

	sptr<Texture> GetTexture() const { return mTexture; }
	Vec2 GetScale() const;

	void SetPosition(float x, float y, float z);
	void SetPosition(const Vec2& pos);
	void SetPosition(const Vec3& pos);

	void SetScale(const Vec2& scale);

	void SetActive(bool val) { mIsActive = val; }
	void SetColor(const Vec3& color);

public:
	void ChangeTexture(rsptr<Texture> texture) { mTexture = texture; }
	bool CheckClick(const Vec2& mousePos);
	virtual void OnClick();

	void AddClickCallback(const std::function<void()> callback) { mClickCallback = callback; }

protected:
	virtual void UpdateShaderVars(rsptr<Texture> texture);
};


class SliderUI : public UI {
private:
	float mMinValue = 0.f;
	float mMaxValue = 1.f;
	float mValue{};			// 0~1 normalize value

public:
	SliderUI(const std::string& textureName, const Vec2& pos, const Vec2& scale);
	virtual ~SliderUI() = default;

public:
	void SetMinMaxValue(float min = 0.f, float max = 1.f) { mMinValue = min, mMaxValue = max; }
	void SetValue(float value) { mValue = value / mMaxValue; }

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
	Button(const std::string& textureName, Vec2 pos, const Vec2& scale);

public:
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


// Canvas 위에 UI를 그리도록 한다.
class Canvas : public Singleton<Canvas> {
	friend Singleton;
	using Layer = int;

	template <typename T>
	static constexpr bool is_valid_ui_type = (std::is_same<T, UI>::value || std::is_same<T, SliderUI>::value || std::is_same<T, Button>::value);

private:
	static constexpr UINT8 mkLayerCnt = 5;
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
	T* CreateUI(Layer layer, const std::string& texture, const Vec2& pos, const Vec2& scale)
	{
		if (layer > (mkLayerCnt - 1))
			return nullptr;

		sptr<T> ui = std::make_shared<T>(texture, pos, scale);
		mUIs[layer].insert(ui);
		return ui.get();
	}
	void RemoveUI(Layer layer, UI* ui);

	void CheckClick(const Vec2& mousePos);
};
#pragma endregion
