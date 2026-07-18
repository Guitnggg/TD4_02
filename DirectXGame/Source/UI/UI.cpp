#include "UI.h"

#include <algorithm>
#include <cassert>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

using namespace KamataEngine;

namespace {
// 1280x720のゲーム画面右上に配置するポーズアイコンの判定範囲。
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
} // namespace

UI::~UI() {
	for (int i = 0; i < 4; i++) {
		delete spritePause_[i];
	}
}

void UI::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	worldTransfrom_.Initialize();
	worldTransfrom_.scale_ = {2, 2, 2};

	textureHandle_[0] = TextureManager::Load("UI/Pause.png");
	textureHandle_[1] = TextureManager::Load("UI/Pose1.png");
	textureHandle_[2] = TextureManager::Load("UI/Pose2.png");
	textureHandle_[3] = TextureManager::Load("UI/Pose3.png");

	Click = audio_->LoadWave("SE/Dicision.mp3");

	spritePause_[0] = Sprite::Create(textureHandle_[0], {1216, 0});
	spritePause_[1] = Sprite::Create(textureHandle_[1], {320, 180});
	spritePause_[2] = Sprite::Create(textureHandle_[2], {320, 180});
	spritePause_[3] = Sprite::Create(textureHandle_[3], {320, 180});

	moveC = 0;
	mouseSelectedItem_ = -1;
	Uiflag = false;
	Stageselect = false;
	Progress = false;
}

void UI::Update() {
	// 毎フレーム、マウス選択をキーボード選択より先に処理する。
	Pause();
	MoveC();

	worldTransfrom_.UpdateMatrix();
}

void UI::Draw() {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Sprite::PreDraw(commandList);
	spritePause_[0]->Draw();

	if (Uiflag && moveC == 0) {
		spritePause_[1]->Draw();
	}
	if (Uiflag && moveC == 1) {
		spritePause_[2]->Draw();
	}
	if (Uiflag && moveC == 2) {
		spritePause_[3]->Draw();
	}

	Sprite::PostDraw();
	dxCommon_->ClearDepthBuffer();
}

void UI::Pause() {
#ifdef USE_IMGUI
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}
#endif
	const Vector2& mousePosition = input_->GetMousePosition();
	const bool clicked = input_->IsTriggerMouse(0);

	if (clicked && IsPauseButton(mousePosition)) {
		// 同じアイコンでポーズメニューの開閉を切り替える。
		audio_->PlayWave(Click, false);
		Uiflag = !Uiflag;
		mouseSelectedItem_ = -1;
		return;
	}

	if (!Uiflag) {
		return;
	}

	const int clickedItem = GetMenuItemIndex(mousePosition);
	if (clickedItem < 0) {
		return;
	}

	moveC = clickedItem;
	mouseSelectedItem_ = clickedItem;
	// ホバーでは選択のみを変更し、クリックされた場合はその場で決定する。
	if (clicked) {
		ExecuteSelectedItem();
		mouseSelectedItem_ = -1;
	}
}

void UI::MoveC() {
	bool moved = false;

	if (input_->TriggerKey(DIK_UP)) {
		moveC -= 1;
		moved = true;
	}
	if (input_->TriggerKey(DIK_DOWN)) {
		moveC += 1;
		moved = true;
	}

	moveC = std::clamp(moveC, 0, kMenuItemCount - 1);

	if (moved) {
		mouseSelectedItem_ = -1;
	}

	if (Uiflag && input_->TriggerKey(DIK_RETURN)) {
		ExecuteSelectedItem();
		mouseSelectedItem_ = -1;
	}
}

void UI::ExecuteSelectedItem() {
	// ここで設定したフラグをGameSceneが参照し、次のシーンを決定する。
	audio_->PlayWave(Click, false);

	if (moveC == 0) {
		Uiflag = false;
		return;
	}
	if (moveC == 1) {
		Stageselect = true;
		return;
	}
	if (moveC == 2) {
		Progress = true;
	}
}

int UI::GetMenuItemIndex(const Vector2& mousePosition) const {
	if (mousePosition.x < kMenuLeft || mousePosition.x > kMenuLeft + kMenuWidth) {
		return -1;
	}
	if (mousePosition.y < kMenuTop || mousePosition.y > kMenuTop + kMenuHeight) {
		return -1;
	}

	const float itemOffsetY = mousePosition.y - kMenuItemTop;
	if (itemOffsetY < 0.0f || itemOffsetY >= kMenuItemHeight * kMenuItemCount) {
		return -1;
	}

	return static_cast<int>(itemOffsetY / kMenuItemHeight);
}

bool UI::IsPauseButton(const Vector2& mousePosition) const {
	return mousePosition.x >= kPauseButtonLeft && mousePosition.x <= kPauseButtonLeft + kPauseButtonSize &&
	       mousePosition.y >= kPauseButtonTop && mousePosition.y <= kPauseButtonTop + kPauseButtonSize;
}
