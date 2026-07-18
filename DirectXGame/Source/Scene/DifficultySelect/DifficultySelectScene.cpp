#include "DifficultySelectScene.h"

#include "../GameScene/GameScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include <memory>

using namespace KamataEngine;

namespace {
constexpr float kDifficultyItemLeft = 430.0f;
constexpr float kDifficultyItemTop = 245.0f;
constexpr float kDifficultyItemWidth = 420.0f;
constexpr float kDifficultyItemHeight = 65.0f;
}

void DifficultySelectScene::Initialize() {
	isEnd_ = false;
	selectedIndex_ = 0;
	const uint32_t whiteTexture = TextureManager::Load("white1x1.png");
	const uint32_t arrowTexture = TextureManager::Load("UI/Arrow.png");
	backgroundSprite_.reset(Sprite::Create(whiteTexture, {0.0f, 0.0f}, {0.04f, 0.08f, 0.13f, 1.0f}));
	if (backgroundSprite_) { backgroundSprite_->SetSize({1280.0f, 720.0f}); }
	panelSprite_.reset(Sprite::Create(whiteTexture, {360.0f, 150.0f}, {0.12f, 0.22f, 0.32f, 0.95f}));
	if (panelSprite_) { panelSprite_->SetSize({560.0f, 440.0f}); }
	cursorSprite_.reset(Sprite::Create(arrowTexture, {430.0f, 280.0f}, {1.0f, 0.65f, 0.15f, 1.0f}, {0.5f, 0.5f}));
	if (cursorSprite_) {
		cursorSprite_->SetSize({42.0f, 42.0f});
		cursorSprite_->SetRotation(1.57079632679f);
	}

	Audio* audio = Audio::GetInstance();
	decisionSoundHandle_ = audio->LoadWave("SE/Dicision.mp3");
}

void DifficultySelectScene::Update() {
	Input* input = Input::GetInstance();
	bool mouseAvailable = true;
#ifdef USE_IMGUI
	mouseAvailable = !ImGui::GetIO().WantCaptureMouse;
#endif
	if (mouseAvailable) {
		const Vector2& mouse = input->GetMousePosition();
		if (mouse.x >= kDifficultyItemLeft && mouse.x <= kDifficultyItemLeft + kDifficultyItemWidth &&
			mouse.y >= kDifficultyItemTop && mouse.y < kDifficultyItemTop + kDifficultyItemHeight * static_cast<float>(kDifficulties.size())) {
			selectedIndex_ = static_cast<int>((mouse.y - kDifficultyItemTop) / kDifficultyItemHeight);
			if (input->IsTriggerMouse(0)) {
				Audio::GetInstance()->PlayWave(decisionSoundHandle_, false, 0.8f);
				isEnd_ = true;
				return;
			}
		}
	}

	if (input->TriggerKey(DIK_W) || input->TriggerKey(DIK_UP)) {
		selectedIndex_ = (selectedIndex_ + static_cast<int>(kDifficulties.size()) - 1) % static_cast<int>(kDifficulties.size());
	}
	if (input->TriggerKey(DIK_S) || input->TriggerKey(DIK_DOWN)) {
		selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(kDifficulties.size());
	}
	if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
		Audio::GetInstance()->PlayWave(decisionSoundHandle_, false, 0.8f);
		isEnd_ = true;
	}
}

void DifficultySelectScene::Draw() {
	if (cursorSprite_) { cursorSprite_->SetPosition({430.0f, 280.0f + 65.0f * static_cast<float>(selectedIndex_)}); }
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	if (backgroundSprite_) { backgroundSprite_->Draw(); }
	if (panelSprite_) { panelSprite_->Draw(); }
	if (cursorSprite_) { cursorSprite_->Draw(); }

	DebugText* debugText = DebugText::GetInstance();
	debugText->Print("SELECT DIFFICULTY", 455.0f, 185.0f, 2.0f);
	for (int i = 0; i < static_cast<int>(kDifficulties.size()); ++i) {
		debugText->Print(kDifficulties[i].name, 480.0f, 265.0f + 65.0f * static_cast<float>(i), 2.0f);
	}
	debugText->Print("MOUSE OR W/S : SELECT", 430.0f, 540.0f, 1.2f);
	debugText->Print("LEFT CLICK OR ENTER : START", 430.0f, 565.0f, 1.2f);
	debugText->DrawAll();
	Sprite::PostDraw();

#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(500.0f, 250.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300.0f, 180.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("DifficultySelectScene");
	ImGui::Text("Select Difficulty");
	ImGui::Separator();
	for (int i = 0; i < static_cast<int>(kDifficulties.size()); ++i) {
		ImGui::Text("%s %s", selectedIndex_ == i ? ">" : " ", kDifficulties[i].name);
	}
	ImGui::Separator();
	ImGui::Text("Mouse or W/S: select");
	ImGui::Text("Left click or SPACE/ENTER: start");
	ImGui::End();
#endif
}

bool DifficultySelectScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> DifficultySelectScene::NextScene() const {
	return std::make_unique<GameScene>(kDifficulties[selectedIndex_].stageFilePath);
}

SceneName DifficultySelectScene::GetSceneName() const { return SceneName::DifficultySelect; }
