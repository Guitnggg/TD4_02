#pragma once
#include "KamataEngine.h"
#include <array>
#include <memory>

/// ゲーム中のポーズボタンとポーズメニューを管理する。
class UI {
public:
	void Initialize();
	void Update();
	void Draw();
	bool ShouldReturnToStageSelect() const { return shouldReturnToStageSelect_; }
	bool ShouldReturnToTitle() const { return shouldReturnToTitle_; }
	bool IsPaused() const { return isPaused_; }

private:
	void UpdatePauseMenu();
	void UpdateKeyboardSelection();
	void ExecuteSelectedItem();
	int GetMenuItemIndex(const KamataEngine::Vector2& mousePosition) const;
	bool IsPauseButton(const KamataEngine::Vector2& mousePosition) const;
	KamataEngine::DirectXCommon* dxCommon_ = nullptr;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Audio* audio_ = nullptr;
	std::array<uint32_t, 4> textureHandles_{};
	std::array<std::unique_ptr<KamataEngine::Sprite>, 4> pauseSprites_{};
	uint32_t decisionSoundHandle_ = 0;
	int selectedMenuItem_ = 0;
	bool isPaused_ = false;
	bool shouldReturnToStageSelect_ = false;
	bool shouldReturnToTitle_ = false;
};
