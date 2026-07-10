#include "DifficultySelectScene.h"

#include "../GameScene/GameScene.h"

#include <KamataEngine.h>
#include <dinput.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include <memory>

using namespace KamataEngine;

void DifficultySelectScene::Initialize() {
	isEnd_ = false;
	selectedIndex_ = 0;
}

void DifficultySelectScene::Update() {
	Input* input = Input::GetInstance();

	if (input->TriggerKey(DIK_W) || input->TriggerKey(DIK_UP)) {
		selectedIndex_ = (selectedIndex_ + static_cast<int>(kDifficulties.size()) - 1) % static_cast<int>(kDifficulties.size());
	}
	if (input->TriggerKey(DIK_S) || input->TriggerKey(DIK_DOWN)) {
		selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(kDifficulties.size());
	}
	if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
		isEnd_ = true;
	}
}

void DifficultySelectScene::Draw() {
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
	ImGui::Text("W/S or UP/DOWN: select");
	ImGui::Text("SPACE/ENTER: start");
	ImGui::End();
#endif
}

bool DifficultySelectScene::IsEnd() const { return isEnd_; }

std::unique_ptr<IScene> DifficultySelectScene::NextScene() const {
	return std::make_unique<GameScene>(kDifficulties[selectedIndex_].stageFilePath);
}

SceneName DifficultySelectScene::GetSceneName() const { return SceneName::DifficultySelect; }
