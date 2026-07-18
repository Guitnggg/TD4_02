#include "ResultScene.h"

#include "../Title/TitleScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
#include <memory>

using namespace KamataEngine;

void ResultScene::Initialize() {
	isEnd_ = false;
	const uint32_t whiteTexture = TextureManager::Load("white1x1.png");
	backgroundSprite_.reset(Sprite::Create(whiteTexture, {0.0f, 0.0f}, {0.03f, 0.10f, 0.08f, 1.0f}));
	if (backgroundSprite_) { backgroundSprite_->SetSize({1280.0f, 720.0f}); }
	panelSprite_.reset(Sprite::Create(whiteTexture, {340.0f, 210.0f}, {0.12f, 0.32f, 0.22f, 1.0f}));
	if (panelSprite_) { panelSprite_->SetSize({600.0f, 300.0f}); }

	Audio* audio = Audio::GetInstance();
	const uint32_t fanfareHandle = audio->LoadWave("fanfare.wav");
	audio->PlayWave(fanfareHandle, false, 0.7f);
}

void ResultScene::Update() {
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isEnd_ = true;
	}
}

void ResultScene::Draw() {
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	if (backgroundSprite_) { backgroundSprite_->Draw(); }
	if (panelSprite_) { panelSprite_->Draw(); }
	DebugText::GetInstance()->Print("STAGE CLEAR!", 445.0f, 285.0f, 3.0f);
	DebugText::GetInstance()->Print("PRESS SPACE TO TITLE", 445.0f, 405.0f, 1.5f);
	DebugText::GetInstance()->DrawAll();
	Sprite::PostDraw();
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240.0f, 120.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("ResultScene");
	ImGui::Text("CLEAR!");
	ImGui::Separator();
	ImGui::Text("Press SPACE to title");
	ImGui::End();
#endif
}

bool ResultScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> ResultScene::NextScene() const {
	return std::make_unique<TitleScene>();
}

SceneName ResultScene::GetSceneName() const { return SceneName::Result; }
