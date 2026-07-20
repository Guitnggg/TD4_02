#include "UI.h"
#include <algorithm>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
using namespace KamataEngine;

namespace {
constexpr float kPauseButtonLeft = 1216.0f;
constexpr float kPauseButtonTop = 0.0f;
constexpr float kPauseButtonSize = 64.0f;
constexpr float kMenuLeft = 320.0f;
constexpr float kMenuTop = 180.0f;
constexpr float kMenuWidth = 640.0f;
constexpr float kMenuHeight = 320.0f;
constexpr float kMenuItemTop = 260.0f;
constexpr float kMenuItemHeight = 80.0f;
constexpr int kMenuItemCount = 3;
}

void UI::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	textureHandles_[0] = TextureManager::Load("UI/Pause.png");
	textureHandles_[1] = TextureManager::Load("UI/Pose1.png");
	textureHandles_[2] = TextureManager::Load("UI/Pose2.png");
	textureHandles_[3] = TextureManager::Load("UI/Pose3.png");
	decisionSoundHandle_ = audio_->LoadWave("SE/Dicision.mp3");
	pauseSprites_[0].reset(Sprite::Create(textureHandles_[0], {1216, 0}));
	pauseSprites_[1].reset(Sprite::Create(textureHandles_[1], {320, 180}));
	pauseSprites_[2].reset(Sprite::Create(textureHandles_[2], {320, 180}));
	pauseSprites_[3].reset(Sprite::Create(textureHandles_[3], {320, 180}));
	selectedMenuItem_ = 0;
	isPaused_ = false;
	shouldReturnToStageSelect_ = false;
	shouldReturnToTitle_ = false;
}

void UI::Update() {
	UpdatePauseMenu();
	UpdateKeyboardSelection();
}

void UI::Draw() {
	Sprite::PreDraw(dxCommon_->GetCommandList());
	pauseSprites_[0]->Draw();
	if (isPaused_) { pauseSprites_[selectedMenuItem_ + 1]->Draw(); }
	Sprite::PostDraw();
	dxCommon_->ClearDepthBuffer();
}

void UI::UpdatePauseMenu() {
#ifdef USE_IMGUI
	if (ImGui::GetIO().WantCaptureMouse) { return; }
#endif
	const Vector2& mousePosition = input_->GetMousePosition();
	const bool clicked = input_->IsTriggerMouse(0);
	if (clicked && IsPauseButton(mousePosition)) {
		audio_->PlayWave(decisionSoundHandle_, false);
		isPaused_ = !isPaused_;
		return;
	}
	if (!isPaused_) { return; }
	const int hoveredItem = GetMenuItemIndex(mousePosition);
	if (hoveredItem >= 0) {
		selectedMenuItem_ = hoveredItem;
		if (clicked) { ExecuteSelectedItem(); }
	}
}

void UI::UpdateKeyboardSelection() {
	if (input_->TriggerKey(DIK_UP)) { --selectedMenuItem_; }
	if (input_->TriggerKey(DIK_DOWN)) { ++selectedMenuItem_; }
	selectedMenuItem_ = std::clamp(selectedMenuItem_, 0, kMenuItemCount - 1);
	if (isPaused_ && input_->TriggerKey(DIK_RETURN)) { ExecuteSelectedItem(); }
}

void UI::ExecuteSelectedItem() {
	audio_->PlayWave(decisionSoundHandle_, false);
	if (selectedMenuItem_ == 0) { isPaused_ = false; }
	else if (selectedMenuItem_ == 1) { shouldReturnToStageSelect_ = true; }
	else { shouldReturnToTitle_ = true; }
}

int UI::GetMenuItemIndex(const Vector2& mousePosition) const {
	if (mousePosition.x < kMenuLeft || mousePosition.x > kMenuLeft + kMenuWidth ||
		mousePosition.y < kMenuTop || mousePosition.y > kMenuTop + kMenuHeight) { return -1; }
	const float itemOffsetY = mousePosition.y - kMenuItemTop;
	if (itemOffsetY < 0.0f || itemOffsetY >= kMenuItemHeight * kMenuItemCount) { return -1; }
	return static_cast<int>(itemOffsetY / kMenuItemHeight);
}

bool UI::IsPauseButton(const Vector2& mousePosition) const {
	return mousePosition.x >= kPauseButtonLeft && mousePosition.x <= kPauseButtonLeft + kPauseButtonSize &&
		mousePosition.y >= kPauseButtonTop && mousePosition.y <= kPauseButtonTop + kPauseButtonSize;
}
